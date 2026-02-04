using System;
using System.Reflection;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;

namespace Eno
{
    public static class ScriptSetup
    {
        [UnmanagedCallersOnly(EntryPoint = "Init")]
        public static void Initialize(IntPtr pUSPath, IntPtr pDLLPath)
        {
            // Conversion to managed string
            string? usPath = Marshal.PtrToStringUni(pUSPath);
            string? dllPath = Marshal.PtrToStringUni(pDLLPath);

            if (string.IsNullOrEmpty(usPath) || string.IsNullOrEmpty(dllPath))
                throw new InvalidOperationException("User scripts or Assembly DLL cannot be found in Initiator Sequence!");

            var hrManager = new HotReloadManager();
            Initiator.SetHRManager(hrManager);
            Initiator.InitSequence(usPath, dllPath);
        }

        [UnmanagedCallersOnly(EntryPoint = "Reload")]
        public static void Reload(IntPtr pUSPath, IntPtr pDLLPath)
        {
            // Conversion to managed string
            string? usPath = Marshal.PtrToStringUni(pUSPath);
            string? dllPath = Marshal.PtrToStringUni(pDLLPath);

            if (string.IsNullOrEmpty(usPath) || string.IsNullOrEmpty(dllPath))
                throw new InvalidOperationException("User scripts or Assembly DLL cannot be found in Initiator Sequence!");

            Initiator.ReloadSequence(usPath, dllPath);
        }

        [UnmanagedCallersOnly(EntryPoint = "GetFileClassNames")]
        public static unsafe IntPtr GetFileClassNames(IntPtr pFilePath, int* count)
        {
            string? filePath = Marshal.PtrToStringUni(pFilePath);
            if (!File.Exists(filePath))
            {
                *count = 0;
                return IntPtr.Zero;
            }

            string code = File.ReadAllText(filePath);
            SyntaxTree tree = CSharpSyntaxTree.ParseText(code);
            var root = tree.GetCompilationUnitRoot();

            var classNames = new List<string>();
            foreach (var classNode in root.DescendantNodes().OfType<ClassDeclarationSyntax>())
            {
                // Get full name including namespace if present
                string fullName = classNode.Identifier.Text;
                var ns = classNode.FirstAncestorOrSelf<NamespaceDeclarationSyntax>();
                if (ns != null)
                    fullName = ns.Name.ToString() + "." + fullName;

                classNames.Add(fullName);
            }

            *count = classNames.Count;

            if (*count == 0)
                return IntPtr.Zero;

            // Allocate LPWSTR pointers            
            IntPtr arrayPtr = Marshal.AllocCoTaskMem(IntPtr.Size * *count);
            for (int i = 0; i < *count; ++i)
            {
                IntPtr pStr = Marshal.StringToCoTaskMemUni(classNames[i]);
                Marshal.WriteIntPtr(arrayPtr, i * IntPtr.Size, pStr);
            }

            return arrayPtr;
        }

        // Retrieve class names from a cs file
        public static string[] GetClassNames(string assemblyPath)
        {
            var assembly = Assembly.LoadFile(assemblyPath);
            var types = assembly.GetTypes();
            var classNames = new List<string>();

            foreach (var type in types)
            {
                if (type.IsClass)
                {
                    classNames.Add(type.FullName!);
                }
            }

            return classNames.ToArray();
        }

        // Use to get script fields, and provide a pointer to the data
        [UnmanagedCallersOnly(EntryPoint = "GetScriptFields")]
        public static unsafe FieldInfoData* GetScriptFields(char* scriptNamePtr, int* count)
        {
            // Convert ptr to string and get fields from script
            string scriptName = new string(scriptNamePtr);
            var fields = GetFields(scriptName);
            *count = fields.Count;

            if (*count == 0)
                return null;

            // Find the number of bytes needed to
            // allocate a continguous block of unmanaged memory
            int size = Marshal.SizeOf<FieldInfoData>();

            // Allocate mem
            IntPtr arrayPtr = Marshal.AllocCoTaskMem(size * *count);

            // Copy each struct into new block of memory
            for (int i = 0; i < *count; i++)
            {
                var field = fields[i];

                // Allocate unmanaged memory for strings
                field.Name = AllocString(Marshal.PtrToStringUni(field.Name) ?? "");
                field.Type = AllocString(Marshal.PtrToStringUni(field.Type) ?? "");
                field.Value = AllocString(Marshal.PtrToStringUni(field.Value) ?? "");
                field.Origin = AllocString(Marshal.PtrToStringUni(field.Origin) ?? "");

                // Handle enums
                if (field.EnumData != IntPtr.Zero)
                {
                    var enumField = (EnumFieldData)Marshal.PtrToStructure(field.EnumData, typeof(EnumFieldData))!;
                    int enumCount = enumField.Count;

                    // Allocate memory for enum names
                    IntPtr namesPtr = Marshal.AllocCoTaskMem(IntPtr.Size * enumCount);
                    for (int j = 0; j < enumCount; j++)
                    {
                        string enumName = Marshal.PtrToStringUni(Marshal.ReadIntPtr(enumField.Names, j * IntPtr.Size)) ?? "";
                        Marshal.WriteIntPtr(namesPtr, j * IntPtr.Size, AllocString(enumName));
                    }
                    enumField.Names = namesPtr;

                    // Allocate memory for enum values
                    IntPtr valuesPtr = Marshal.AllocCoTaskMem(sizeof(int) * enumCount);
                    for (int j = 0; j < enumCount; j++)
                    {
                        int value = Marshal.ReadInt32(enumField.Values, j * sizeof(int));
                        Marshal.WriteInt32(valuesPtr, j * sizeof(int), value);
                    }
                    enumField.Values = valuesPtr;

                    // Allocate memory for the EnumFieldData struct itself
                    IntPtr enumDataPtr = Marshal.AllocCoTaskMem(Marshal.SizeOf<EnumFieldData>());
                    Marshal.StructureToPtr(enumField, enumDataPtr, false);
                    field.EnumData = enumDataPtr;
                }

                // Copy FieldInfoData into the unmanaged array
                IntPtr structPtr = arrayPtr + i * size;
                Marshal.StructureToPtr(field, structPtr, false);
            }

            return (FieldInfoData*)arrayPtr;
        }

        private static IntPtr AllocString(string str)
        {
            if (str == null) return IntPtr.Zero;
            return Marshal.StringToCoTaskMemUni(str);
        }

        // Create field data based on number of fields
        // and return a list of these data
        public static List<FieldInfoData> GetFields(string scriptName)
        {
            // Determine type of field
            var type = Type.GetType($"Eno.{scriptName}");
            var result = new List<FieldInfoData>();
            if (type == null) 
                return result;

            // Creates an instance of the type of field
            var instance = Activator.CreateInstance(type);

            // Iterate over public and non-public fields to reveal
            foreach (var field in type.GetFields(BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic))
            {
                // Check if field has [Reveal] attribute
                // Only fields annotated with [Reveal] will be added
                var reveal = field.GetCustomAttribute<RevealAttribute>();
                if (reveal == null)
                    continue;

                var data = new FieldInfoData
                {
                    // Value is set to null if there is no value
                    // Use StringToCoTaskMemUni to allocate a block of unmanaged memory to hold a string of UTF-16
                    // Copy the managed string to the unamanaged memory
                    // Return an IntPtr to that unmanaged memory block
                    Name = Marshal.StringToCoTaskMemUni(field.Name),
                    Type = Marshal.StringToCoTaskMemUni(field.FieldType.Name),
                    Value = Marshal.StringToCoTaskMemUni(field.GetValue(instance)?.ToString() ?? "null"),
                    Origin = Marshal.StringToCoTaskMemUni(field.DeclaringType?.FullName ?? "Unknown"),
                    Min = reveal.Min,
                    Max = reveal.Max,
                    HasRange = reveal.HasRange
                };
                
                // Enum handling
                if (field.FieldType.IsEnum)
                {
                    var names = Enum.GetNames(field.FieldType);
                    var values = Enum.GetValues(field.FieldType);

                    var enumField = new EnumFieldData
                    {
                        Count = names.Length,
                        Names = IntPtr.Zero,
                        Values = IntPtr.Zero,
                        ValueSize = 0
                    };

                    IntPtr namesPtr = Marshal.AllocCoTaskMem(IntPtr.Size * names.Length);
                    for (int i = 0; i < names.Length; ++i)
                    {
                        Marshal.WriteIntPtr(namesPtr, i * IntPtr.Size, Marshal.StringToCoTaskMemUni(names[i]));
                    }
                    enumField.Names = namesPtr;

                    IntPtr valuesPtr = Marshal.AllocCoTaskMem(sizeof(int) * values.Length);
                    Type underlyingType = Enum.GetUnderlyingType(field.FieldType);
                    int valueSize = underlyingType switch
                    {
                        Type t when t == typeof(byte) => 1,
                        Type t when t == typeof(short) => 2,
                        Type t when t == typeof(int) => 4,
                        Type t when t == typeof(long) => 8,
                        _ => throw new Exception("Unsupported enum type")
                    };

                    for (int j = 0; j < values.Length; j++)
                    {
                        object val = values.GetValue(j)!;
                        switch (valueSize)
                        {
                            case 1: Marshal.WriteByte(valuesPtr, j, (byte)val); break;
                            case 2: Marshal.WriteInt16(valuesPtr, j * 2, (short)val); break;
                            case 4: Marshal.WriteInt32(valuesPtr, j * 4, (int)val); break;
                            case 8: Marshal.WriteInt64(valuesPtr, j * 8, (long)val); break;
                        }
                    }
                    enumField.Values = valuesPtr;

                    IntPtr enumDataPtr = Marshal.AllocCoTaskMem(Marshal.SizeOf<EnumFieldData>());
                    Marshal.StructureToPtr(enumField, enumDataPtr, false);
                    data.EnumData = enumDataPtr;
                }
                else
                {
                    data.EnumData = IntPtr.Zero;
                }

                result.Add(data);
            }

            return result;
        }
    }
}
