using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace NoEye_Setup
{
    class NoEye32
    {
        [DllImport("Service\\NoEye.dll")]
        public static extern int ServiceInstaller_InstallService(string lpServiceName, string lpDisplayName, string lpPath);
        [DllImport("Service\\NoEye.dll")]
        public static extern int ServiceInstaller_UninstallService(string lpServiceName);
        [DllImport("Service\\NoEye.dll")]
        public static extern int ServiceInstaller_GetServiceStatus(string lpServiceName);
    }
    class NoEye64
    {
        [DllImport("Service\\NoEye64.dll")]
        public static extern int ServiceInstaller_InstallService(string lpServiceName, string lpDisplayName, string lpPath);
        [DllImport("Service\\NoEye64.dll")]
        public static extern int ServiceInstaller_UninstallService(string lpServiceName);
        [DllImport("Service\\NoEye64.dll")]
        public static extern int ServiceInstaller_GetServiceStatus(string lpServiceName);
    }

}
