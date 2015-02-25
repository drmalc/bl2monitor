using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Timers;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.IO;
using System.IO.Pipes;
using System.Threading;
using System.Text.RegularExpressions;

namespace bl2_monitor
{
    public partial class Form1 : Form
    {
        //String constants
        private const String process_name = "Borderlands2";
        private const String dll_name = "libbl2monitor.dll";
        private const String pipe_name = "\\\\.\\pipe\\bl2monitorpipe";
        private const String log_file_name = @"C:\temp\bl2monitor.log"; //Not used by default. Feel free to change.

        //Log
        private String logBuffer;
        private System.IO.StreamWriter log_file;
        private bool log_to_file = false; //log to a file instead of the console. Usefull for the debug messages. Produces large files, be careful.
        private bool show_debug_in_console = false; //not recommended, significantly slows down the game.

        //Other
        private bool targetReady = false;
        private int processId = 0;
        private CSNamedPipe.NamedPipeServer PServer1;
        private System.Timers.Timer statusTimer;
        private bool timer_exec = false; //FIXME. Awful workaround to make sure the timer is stopped as the form closes. Will make the user click Close twice sometimes.

        //kernel32 stuff
        const uint PROCESS_CREATE_THREAD = 0x2;
        const uint PROCESS_VM_OPERATION = 0x8;
        const uint PROCESS_VM_WRITE = 0x0020;
        const uint PROCESS_QUERY_INFORMATION = 0x400;
        const uint PROCESS_VM_READ = 0x0010;
        const uint MEM_COMMIT = 0x00001000;
        const uint MEM_RESERVE = 0x00002000;
        const uint PAGE_EXECUTE_READWRITE = 0x40;
        [DllImport("kernel32.dll")]
        private static extern int CloseHandle(IntPtr hObject);
        [DllImport("kernel32.dll")]
        private static extern IntPtr GetProcAddress(IntPtr hModule, string procedureName);
        [DllImport("kernel32.dll")]
        private static extern IntPtr GetModuleHandle(string lpModuleName);
        [DllImport("kernel32.dll")]
        private static extern IntPtr OpenProcess(uint dwDesiredAccess, int bInheritHandle, int dwProcessId);
        [DllImport("kernel32.dll")]
        private static extern IntPtr VirtualAllocEx(IntPtr hProcess, IntPtr lpAddress, IntPtr dwSize, uint flAllocationType, uint flProtect);
        [DllImport("kernel32.dll")]
        private static extern int WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] buffer, uint size, int lpNumberOfBytesWritten);
        [DllImport("kernel32.dll")]
        private static extern IntPtr CreateRemoteThread(IntPtr hProcess, IntPtr lpThreadAttribute, IntPtr dwStackSize, IntPtr lpStartAddress, IntPtr lpParameter, uint dwCreationFlags, IntPtr lpThreadId);

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_FormClosing(Object sender, FormClosingEventArgs e)
        {
            PServer1.StopServer();

            statusTimer.Stop();
            if (timer_exec)//FIXME properly invalidate the timer.
            {
                e.Cancel = true;
                return;
            }

            if (log_to_file)
                log_file.Close();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            statusTimer = new System.Timers.Timer(500);
            statusTimer.Elapsed += new ElapsedEventHandler(update_timer);
            statusTimer.AutoReset = false;

            //Create log file
            if (log_to_file)
                log_file = new System.IO.StreamWriter(log_file_name);

            //Create local named pipe
            CSNamedPipe.DataReceivedCallbackType cb = new CSNamedPipe.DataReceivedCallbackType(this.pipe_received_data);

            PServer1 = new CSNamedPipe.NamedPipeServer(pipe_name, 0);
            PServer1.Start(cb);

            //Start the status timer
            statusTimer.Enabled = true;
        }

        //Callback
        public void pipe_received_data(String str)
        {
            str = Regex.Replace(str, @"[^\u0001-\u007F]", string.Empty); //Remove non-ascii characters.
            logBuffer += str;

            int index = -1;
            while ((index = logBuffer.IndexOf("\n")) >= 0)
            {
                str = logBuffer.Substring(0, index) + Environment.NewLine;
                logBuffer = logBuffer.Substring(index+1);

                if (log_to_file)
                {
                    log_file.Write(str);
                }
                else
                {
                    if (show_debug_in_console || !str.Contains("DEBG"))
                    {
                        Invoke(new MethodInvoker(() =>
                        {
                            logTextBox.AppendText(str);
                        }));
                    }
                }
            }
        }

        private void update_timer(object sender, ElapsedEventArgs e)
        {
            timer_exec = true;
            Invoke(new MethodInvoker(() =>
            {
                if (targetReady)
                {
                    try
                    {
                        Process.GetProcessById(processId);
                    }
                    catch (Exception)
                    {
                        //Target is no longer running.
                        targetReady = false;
                        statusLabel.Text = "Target terminated.";
                    }
                }
                else
                {
                    statusLabel.Text = "Waiting for target...";

                    Process[] processes = Process.GetProcessesByName(process_name);
                    if (processes.Length == 1)
                    {
                        statusLabel.Text = "Target found.";
                        Process process = processes.FirstOrDefault();
                        processId = process.Id;

                        Thread.Sleep(100); //Just for safety. Not needed.

                        Process currentProc = Process.GetCurrentProcess();
                        String procPath = currentProc.MainModule.FileName;
                        String dirPath = Directory.GetParent(procPath).FullName;
                        String dllPath = Directory.GetFiles(dirPath, dll_name)[0];

                        IntPtr procPtr = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_READ | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION, 0, processId);

                        if (procPtr != (IntPtr)0)
                        {
                            IntPtr loadPtr = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
                            if (loadPtr != (IntPtr)0)
                            {
                                IntPtr lpAddress = VirtualAllocEx(procPtr, (IntPtr)null, (IntPtr)dllPath.Length, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
                                if (lpAddress != (IntPtr)0)
                                {
                                    byte[] bytes = Encoding.ASCII.GetBytes(dllPath);
                                    WriteProcessMemory(procPtr, lpAddress, bytes, (uint)bytes.Length, 0);

                                    IntPtr remoteThread = CreateRemoteThread(procPtr, (IntPtr)null, (IntPtr)0, loadPtr, lpAddress, 0, (IntPtr)null);

                                    CloseHandle(procPtr);
                                    statusLabel.Text = "Ready.";
                                    targetReady = true;
                                }
                            }
                        }
                    }
                }
            }));
            timer_exec = false;

            statusTimer.Enabled = true;
        }
    }
}
