import tkinter as tk
from tkinter import ttk, messagebox
import serial
import serial.tools.list_ports
import threading
import time

class SmartBLDCDashboard:
    def __init__(self, root):
        self.root = root
        self.root.title("Smart BLDC Servo Control")
        self.root.geometry("600x500")
        self.root.configure(bg="#1E1E2E")
        
        self.style = ttk.Style()
        self.style.theme_use('clam')
        self.style.configure("TFrame", background="#1E1E2E")
        self.style.configure("TLabel", background="#1E1E2E", foreground="#CDD6F4", font=("Inter", 12))
        self.style.configure("TButton", background="#89B4FA", foreground="#1E1E2E", font=("Inter", 12, "bold"))
        self.style.map("TButton", background=[("active", "#B4BEFE")])
        
        self.serial_port = None
        self.is_connected = False
        
        self.build_ui()

    def build_ui(self):
        # Header
        header = tk.Label(self.root, text="Smart BLDC Servo", font=("Inter", 24, "bold"), bg="#1E1E2E", fg="#89B4FA")
        header.pack(pady=20)
        
        # Connection Frame
        conn_frame = ttk.Frame(self.root)
        conn_frame.pack(pady=10, fill="x", padx=40)
        
        ttk.Label(conn_frame, text="Port:").pack(side="left", padx=5)
        self.port_cb = ttk.Combobox(conn_frame, values=[p.device for p in serial.tools.list_ports.comports()], state="readonly")
        self.port_cb.pack(side="left", padx=5, expand=True, fill="x")
        
        self.btn_connect = ttk.Button(conn_frame, text="Connect", command=self.toggle_connection)
        self.btn_connect.pack(side="left", padx=5)
        
        # Control Frame
        ctrl_frame = ttk.Frame(self.root)
        ctrl_frame.pack(pady=20, fill="both", expand=True, padx=40)
        
        ttk.Label(ctrl_frame, text="Target Position (deg):").grid(row=0, column=0, pady=10, sticky="w")
        self.target_pos = tk.DoubleVar(value=0.0)
        self.slider_pos = ttk.Scale(ctrl_frame, from_=-360, to=360, variable=self.target_pos, orient="horizontal", command=self.update_pos_label)
        self.slider_pos.grid(row=0, column=1, pady=10, sticky="ew", padx=10)
        self.lbl_pos_val = ttk.Label(ctrl_frame, text="0.0°")
        self.lbl_pos_val.grid(row=0, column=2, pady=10)
        
        # Actions
        btn_send = ttk.Button(ctrl_frame, text="Send Command", command=self.send_position)
        btn_send.grid(row=1, column=0, columnspan=3, pady=20, sticky="ew")
        
        self.lbl_status = ttk.Label(self.root, text="Status: Disconnected", foreground="#F38BA8")
        self.lbl_status.pack(side="bottom", pady=20)
        
        ctrl_frame.columnconfigure(1, weight=1)

    def update_pos_label(self, val):
        self.lbl_pos_val.config(text=f"{float(val):.1f}°")

    def toggle_connection(self):
        if not self.is_connected:
            port = self.port_cb.get()
            if not port:
                messagebox.showerror("Error", "Please select a port.")
                return
            try:
                self.serial_port = serial.Serial(port, 1000000, timeout=1)
                self.is_connected = True
                self.btn_connect.config(text="Disconnect")
                self.lbl_status.config(text=f"Status: Connected to {port}", foreground="#A6E3A1")
            except Exception as e:
                messagebox.showerror("Connection Error", str(e))
        else:
            self.serial_port.close()
            self.is_connected = False
            self.btn_connect.config(text="Connect")
            self.lbl_status.config(text="Status: Disconnected", foreground="#F38BA8")

    def send_position(self):
        if not self.is_connected:
            messagebox.showwarning("Warning", "Not connected to any port.")
            return
        
        pos = self.target_pos.get()
        # Mock Feetech Packet generation (Simplified)
        # ID 1, Write Data, Goal Position
        # In a real scenario, proper packet framing with checksum is required.
        packet = f"POS:{pos:.2f}\n".encode()
        try:
            self.serial_port.write(packet)
            self.lbl_status.config(text=f"Sent: {pos:.1f}°")
        except Exception as e:
            messagebox.showerror("Communication Error", str(e))

if __name__ == "__main__":
    root = tk.Tk()
    app = SmartBLDCDashboard(root)
    root.mainloop()
