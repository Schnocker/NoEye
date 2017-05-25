namespace NoEye_Setup
{
    partial class NoEye_GUI
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(NoEye_GUI));
            this.g_license = new System.Windows.Forms.Label();
            this.g_Install = new System.Windows.Forms.Button();
            this.g_Uninstall = new System.Windows.Forms.Button();
            this.g_Link = new System.Windows.Forms.LinkLabel();
            this.g_PBar = new System.Windows.Forms.ProgressBar();
            this.g_Install_percent = new System.Windows.Forms.Label();
            this.g_Status = new System.Windows.Forms.Label();
            this.g_Logo = new System.Windows.Forms.PictureBox();
            this.g_Timer = new System.Windows.Forms.Timer(this.components);
            ((System.ComponentModel.ISupportInitialize)(this.g_Logo)).BeginInit();
            this.SuspendLayout();
            // 
            // g_license
            // 
            this.g_license.AutoSize = true;
            this.g_license.Location = new System.Drawing.Point(188, 9);
            this.g_license.Name = "g_license";
            this.g_license.Size = new System.Drawing.Size(404, 221);
            this.g_license.TabIndex = 1;
            this.g_license.Text = resources.GetString("g_license.Text");
            // 
            // g_Install
            // 
            this.g_Install.Enabled = false;
            this.g_Install.Location = new System.Drawing.Point(235, 242);
            this.g_Install.Name = "g_Install";
            this.g_Install.Size = new System.Drawing.Size(131, 28);
            this.g_Install.TabIndex = 3;
            this.g_Install.Text = "Install";
            this.g_Install.UseVisualStyleBackColor = true;
            this.g_Install.Click += new System.EventHandler(this.g_Install_Click);
            // 
            // g_Uninstall
            // 
            this.g_Uninstall.Enabled = false;
            this.g_Uninstall.Location = new System.Drawing.Point(408, 242);
            this.g_Uninstall.Name = "g_Uninstall";
            this.g_Uninstall.Size = new System.Drawing.Size(131, 28);
            this.g_Uninstall.TabIndex = 4;
            this.g_Uninstall.Text = "Uninstall";
            this.g_Uninstall.UseVisualStyleBackColor = true;
            this.g_Uninstall.Click += new System.EventHandler(this.g_Uninstall_Click);
            // 
            // g_Link
            // 
            this.g_Link.AutoSize = true;
            this.g_Link.Location = new System.Drawing.Point(13, 253);
            this.g_Link.Name = "g_Link";
            this.g_Link.Size = new System.Drawing.Size(145, 13);
            this.g_Link.TabIndex = 6;
            this.g_Link.TabStop = true;
            this.g_Link.Text = "(c) 2017  Schnocker - UC.me";
            this.g_Link.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.g_Link_LinkClicked);
            // 
            // g_PBar
            // 
            this.g_PBar.Location = new System.Drawing.Point(66, 195);
            this.g_PBar.Name = "g_PBar";
            this.g_PBar.Size = new System.Drawing.Size(108, 23);
            this.g_PBar.TabIndex = 7;
            // 
            // g_Install_percent
            // 
            this.g_Install_percent.AutoSize = true;
            this.g_Install_percent.Font = new System.Drawing.Font("Microsoft Sans Serif", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.g_Install_percent.Location = new System.Drawing.Point(13, 195);
            this.g_Install_percent.Name = "g_Install_percent";
            this.g_Install_percent.Size = new System.Drawing.Size(29, 18);
            this.g_Install_percent.TabIndex = 8;
            this.g_Install_percent.Text = "0%";
            // 
            // g_Status
            // 
            this.g_Status.AutoSize = true;
            this.g_Status.Location = new System.Drawing.Point(13, 221);
            this.g_Status.Name = "g_Status";
            this.g_Status.Size = new System.Drawing.Size(119, 13);
            this.g_Status.TabIndex = 9;
            this.g_Status.Text = "Waiting for any action...";
            // 
            // g_Logo
            // 
            this.g_Logo.Image = ((System.Drawing.Image)(resources.GetObject("g_Logo.Image")));
            this.g_Logo.Location = new System.Drawing.Point(13, 9);
            this.g_Logo.Name = "g_Logo";
            this.g_Logo.Size = new System.Drawing.Size(161, 180);
            this.g_Logo.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.g_Logo.TabIndex = 2;
            this.g_Logo.TabStop = false;
            // 
            // g_Timer
            // 
            this.g_Timer.Interval = 2000;
            this.g_Timer.Tick += new System.EventHandler(this.g_Timer_Tick);
            // 
            // NoEye_GUI
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(600, 275);
            this.Controls.Add(this.g_Status);
            this.Controls.Add(this.g_Install_percent);
            this.Controls.Add(this.g_PBar);
            this.Controls.Add(this.g_Link);
            this.Controls.Add(this.g_Uninstall);
            this.Controls.Add(this.g_Install);
            this.Controls.Add(this.g_Logo);
            this.Controls.Add(this.g_license);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "NoEye_GUI";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "NoEye Setup";
            this.Load += new System.EventHandler(this.NoEye_GUI_Load);
            ((System.ComponentModel.ISupportInitialize)(this.g_Logo)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Label g_license;
        private System.Windows.Forms.PictureBox g_Logo;
        private System.Windows.Forms.Button g_Install;
        private System.Windows.Forms.Button g_Uninstall;
        private System.Windows.Forms.LinkLabel g_Link;
        private System.Windows.Forms.ProgressBar g_PBar;
        private System.Windows.Forms.Label g_Install_percent;
        private System.Windows.Forms.Label g_Status;
        private System.Windows.Forms.Timer g_Timer;
    }
}

