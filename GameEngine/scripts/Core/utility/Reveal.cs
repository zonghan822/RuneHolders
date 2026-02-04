using System;
using System.Runtime.InteropServices;

namespace Eno
{
    // Define custom attribute type
    // Will use [Reveal] as syntax
    // Source: https://learn.microsoft.com/en-us/dotnet/api/system.attribute?view=net-9.0
    //         https://learn.microsoft.com/en-us/dotnet/csharp/advanced-topics/reflection-and-attributes/
    [AttributeUsage(AttributeTargets.Field)]
    public class RevealAttribute : Attribute
    {
        // Properties to reveal to the engine
        // Getters / Setters
        public string Description { get; set; } = string.Empty;
        public float Min { get; set; } = 0f;
        public float Max { get; set; } = 0f;
        public bool HasRange { get; set; } = false;

        // Constructors
        public RevealAttribute() {}

        // Min Max constructor
        public RevealAttribute(float min, float max)
        {
            Min = min;
            Max = max;
            HasRange = true;
        }

        // Description constructor
        public RevealAttribute(string desc)
        {
            Description = desc;
        }
    }

    // Specifies memory layout of data to be passed back into the Eno Engine!
    // Used for interop, needs to match
    // FieldInfoData specified in eno_core_host.hpp!!!
    //
    // Source: https://learn.microsoft.com/en-us/dotnet/api/system.intptr?view=net-9.0
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct FieldInfoData
    {
        // Do not use string for these 3 params!
        // They will become unmanaged objects when
        // passed to engine side
        // IntPtr is appropriate for pointing to unmanaged memory!

        // Default field descriptors
        public IntPtr Name;
        public IntPtr Type;
        public IntPtr Value;

        // Optional field properties
        public float Min;
        public float Max;
        
        [MarshalAs(UnmanagedType.Bool)]     // MarshalAs to represent the bool as 4-byte
        public bool HasRange;

        // Origin (Namespace.Class)
        public IntPtr Origin;

        // Enum metadata
        public IntPtr EnumData;
    }

    // For enums
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct EnumFieldData
    {
        public int Count;       // Number of enum entries
        public IntPtr Names;
        public IntPtr Values;
        public int ValueSize;   // An enum may represent a byte, int, or other arithemetic types
    }
}