using System;
using System.Runtime.InteropServices;

namespace Eno
{
    // Define custom attribute type
    // Will use [Reveal] as syntax
    // Source: https://learn.microsoft.com/en-us/dotnet/api/system.attribute?view=net-9.0
    //         https://learn.microsoft.com/en-us/dotnet/csharp/advanced-topics/reflection-and-attributes/
    [AttributeUsage(AttributeTargets.Field)]
    public class PortAttribute : Attribute
    {
        // Properties to reveal to the engine
        // Getters / Setters
        public string Description { get; set; } = string.Empty;
        public float Min { get; set; }
        public float Max { get; set; }
        public bool HasRange { get; set; } = false;

        // Constructors
        public PortAttribute() {}

        // Min Max constructor
        public PortAttribute(float min, float max)
        {
            Min = min;
            Max = max;
            HasRange = true;
        }

        // Description constructor
        public PortAttribute(string desc)
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

    }
}