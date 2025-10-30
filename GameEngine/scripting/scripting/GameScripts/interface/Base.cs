using System;
using System.IO;
using System.Reflection;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;

namespace Eno
{
    public class ScriptBase
    {
        private readonly string name;

        protected ScriptBase([CallerFilePath] string filePath = "")
        {
            name = Path.GetFileNameWithoutExtension(filePath);
        }

        [UnmanagedCallersOnly(EntryPoint = "GetFileClassNames")]
        public static unsafe IntPtr GetFileClassNames(IntPtr pFilePath, int* count)
        {
            string filePath = Marshal.PtrToStringUni(pFilePath);
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
                    classNames.Add(type.FullName);
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
                IntPtr structPtr = arrayPtr + (i * size);
                Marshal.StructureToPtr(fields[i], structPtr, false);
            }

            // Cast to a FieldInfoData pointer for the C++ side
            return (FieldInfoData*)arrayPtr;
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
                    NamspClass = Marshal.StringToCoTaskMemUni(field.DeclaringType.FullName ?? "Unknown"),
                    Min = reveal.Min,
                    Max = reveal.Max,
                    HasRange = reveal.HasRange
                };

                result.Add(data);
            }

            return result;
        }
    }
}
