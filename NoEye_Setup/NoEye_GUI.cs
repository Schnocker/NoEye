using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace NoEye_Setup
{
    public partial class NoEye_GUI : Form
    {
        public NoEye_GUI()
        {
            InitializeComponent();

        }

        private void NoEye_GUI_Load(object sender, EventArgs e)
        {
            try
            {
                int Result = 0;
                if (IntPtr.Size == 8)
                    Result = NoEye64.ServiceInstaller_GetServiceStatus("AmdK9");
                else
                    Result = NoEye32.ServiceInstaller_GetServiceStatus("AmdK9");
                if (Result == 0)
                {
                    MessageBox.Show("ServiceInstaller_GetServiceStatus failed with errorcode " + Result.ToString() + "\nPlease contact the program owner.", "ERROR", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }
                if (Result == 1 || Result == (int)(-1))
                {
                    g_Uninstall.Enabled = false;
                    g_Install.Enabled = true;
                }
                else
                {
                    g_Uninstall.Enabled = true;
                    g_Install.Enabled = false;
                }
                g_Timer.Start();
            }

            catch
            {
                MessageBox.Show("NoEye64 or NoEye not found" + "\nPlease contact the program owner.", "ERROR", MessageBoxButtons.OK, MessageBoxIcon.Error);
                System.Diagnostics.Process.Start("https://www.unknowncheats.me/forum/members/1854603.html");
                Environment.Exit(1);
            }

        }

        private void g_Install_Click(object sender, EventArgs e)
        {
            g_Uninstall_Click(sender, e);
            try
            {
                int Result = 0;
                g_Status.Text = "Installing...";
                g_Install_percent.Text = "25%";
                g_PBar.Value = 25;
                if (IntPtr.Size == 8)
                    Result = NoEye64.ServiceInstaller_InstallService("AmdK9", "AMD K9-Prozessortreiber", Directory.GetCurrentDirectory() + "\\Service\\NoEye64.exe");
                else
                    Result = NoEye32.ServiceInstaller_InstallService("AmdK9", "AMD K9-Prozessortreiber", Directory.GetCurrentDirectory() + "\\Service\\NoEye.exe");
                g_Install_percent.Text = "75%";
                g_PBar.Value = 75;
                if (Result == 0)
                {
                    g_Install_percent.Text = "100%";
                    g_PBar.Value = 100;
                    g_Status.ForeColor = Color.Black;
                    g_Status.Text = "Service has been installed!";
                    return;
                }
                g_Status.Text = "Failed to install the service (" + Result.ToString() + ")";
                g_Status.ForeColor = Color.Red;
            }
            catch
            {
                g_Status.Text = "Failed to install the service (" + "205" + ")";
                g_Status.ForeColor = Color.Red;
            }
        }

        private void g_Uninstall_Click(object sender, EventArgs e)
        {
            try
            {
                int Result = 0, StatusResult = 0;
                g_Status.Text = "Uninstalling...";
                g_Install_percent.Text = "25%";
                g_PBar.Value = 25;
                if (IntPtr.Size == 8)
                {
                    Result = NoEye64.ServiceInstaller_UninstallService("AmdK9");
                    StatusResult = NoEye64.ServiceInstaller_GetServiceStatus("AmdK9");
                }
                else
                {
                    Result = NoEye32.ServiceInstaller_UninstallService("AmdK9");
                    StatusResult = NoEye32.ServiceInstaller_GetServiceStatus("AmdK9");
                }
                g_Install_percent.Text = "75%";
                g_PBar.Value = 75;
                if (StatusResult == 0)
                {
                    MessageBox.Show("ServiceInstaller_GetServiceStatus failed with errorcode " + Result.ToString() + "\nPlease contact the program owner.", "ERROR", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }
                if (Result == 0 && StatusResult == (int)-1 ||
                    Result == 0 && StatusResult == 1)
                {
                    g_Status.ForeColor = Color.Black;
                    g_Install_percent.Text = "100%";
                    g_PBar.Value = 100;
                    g_Status.Text = "Service has been uninstalled!";
                    return;
                }
                g_Status.Text = "Failed to uninstall the service (" + Result.ToString() + " : " + StatusResult.ToString() + ")";
                g_Status.ForeColor = Color.Red;
            }
            catch
            {
                g_Status.Text = "Failed to uninstall the service (" + "206" + ")";
                g_Status.ForeColor = Color.Red;
            }
        }

        private void g_Link_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            System.Diagnostics.Process.Start("https://www.unknowncheats.me/forum/members/1854603.html");
        }

        private void g_Timer_Tick(object sender, EventArgs e)
        {

            int Result = 0;
            if (IntPtr.Size == 8)
                Result = NoEye64.ServiceInstaller_GetServiceStatus("AmdK9");
            else
                Result = NoEye32.ServiceInstaller_GetServiceStatus("AmdK9");
            if (Result == 0)
            {
                g_Timer.Stop();
                MessageBox.Show("ServiceInstaller_GetServiceStatus failed with errorcode " + Result.ToString() + "\nPlease contact the program owner.", "ERROR", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
            if (Result == 1 || Result == (int)(-1))
            {
                g_Uninstall.Enabled = false;
                g_Install.Enabled = true;
            }
            else
            {
                g_Uninstall.Enabled = true;
                g_Install.Enabled = false;
            }
        }
    }
}
