using System;
using System.Runtime.InteropServices;

namespace Eno
{
    // Define custom attribute type
    // Will use [Reveal] as syntax
    [AttributeUsage(AttributeTargets.Field)]
    public class RevealAttribute : Attribute
    {
        // Properties to reveal to the engine
        // Getters / Setters
        public string Description { get; set; } = string.Empty;
        public float Min { get; set; }
        public float Max { get; set; }
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
        public IntPtr NamspClass;

    }
}