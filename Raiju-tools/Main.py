#!/usr/bin/env python

import wx
import wx.adv
import wx.lib.inspection
import wx.lib.mixins.inspection
import sys
import os
import esptool
import threading
import json
import images as images
from serial import SerialException
from serial.tools import list_ports
import locale
import serial


# see https://discuss.wxpython.org/t/wxpython4-1-1-python3-8-locale-wxassertionerror/35168
locale.setlocale(locale.LC_ALL, 'C')

__version__ = "0.1.2_2"

__auto_select__ = "Auto-select"
__supported_baud_rates__ = [9600, 57600, 74880, 115200, 230400, 460800, 921600]

# ---------------------------------------------------------------------------


# See discussion at http://stackoverflow.com/q/41101897/131929
class RedirectText:
    def __init__(self, text_ctrl):
        self.__out = text_ctrl

    def write(self, string):
        if string.startswith("\r"):
            # carriage return -> remove last line i.e. reset position to start of last line
            current_value = self.__out.GetValue()
            last_newline = current_value.rfind("\n")
            new_value = current_value[:last_newline + 1]  # preserve \n
            new_value += string[1:]  # chop off leading \r
            wx.CallAfter(self.__out.SetValue, new_value)
        else:
            wx.CallAfter(self.__out.AppendText, string)

    # noinspection PyMethodMayBeStatic
    def flush(self):
        # noinspection PyStatementEffect
        None

    # esptool >=3 handles output differently of the output stream is not a TTY
    # noinspection PyMethodMayBeStatic
    def isatty(self):
        return True

# ---------------------------------------------------------------------------


# ---------------------------------------------------------------------------
class FlashingThread(threading.Thread):

    def __init__(self, parent, config):
        threading.Thread.__init__(self)
        self.daemon = True
        self._parent = parent
        self._config = config
        self._panel2 = None

    def run(self):
        try:
            command = []

            if not self._config.port.startswith(__auto_select__):
                command.append("--port")
                command.append(self._config.port)

            command.extend(["--baud", str(self._config.baud),
                            "--after", "no_reset",
                            "write_flash",
                            # https://github.com/espressif/esptool/issues/599
                            "--flash_size", "detect",
                            "--flash_mode", self._config.mode,
                            "0x00000", self._config.firmware_path])

            if self._config.erase_before_flash:
                command.append("--erase-all")

            print("Command: esptool.py %s\n" % " ".join(command))

            esptool.main(command)

            # The last line printed by esptool is "Staying in bootloader." -> some indication that the process is
            # done is needed
            print("\nFirmware successfully flashed. Unplug/replug or reset device \nto switch back to normal boot "
                  "mode.")
            self._panel2.on_clicked(None)

        except SerialException as e:
            self._parent.report_error(e.strerror)
            raise e


# ---------------------------------------------------------------------------


# ---------------------------------------------------------------------------
# DTO between GUI and flashing thread
class FlashConfig:
    _instance = None

    def __init__(self):
        if FlashConfig._instance is not None:
            raise Exception(
                "FlashConfig is a Singleton class. Use FlashConfig.getInstance() to get the instance.")
        self.baud = 921600  # 1843200
        self.erase_before_flash = False
        self.mode = "qio"
        self.firmware_path = None
        self.port = __auto_select__

    @classmethod
    def getInstance(cls):
        if cls._instance is None:
            cls._instance = cls()
        return cls._instance

    @classmethod
    def load(cls, file_path):
        conf = cls.getInstance()

        if os.path.exists(file_path):
            with open(file_path, 'r') as f:
                data = json.load(f)
            conf.port = data['port']
            conf.baud = data['baud']
            conf.mode = data['mode']
            conf.erase_before_flash = data['erase']
        return conf

    def safe(self, file_path):
        data = {
            'port': self.port,
            'baud': self.baud,
            'mode': self.mode,
            'erase': self.erase_before_flash,
        }
        with open(file_path, 'w') as f:
            json.dump(data, f)

    def is_complete(self):
        return self.firmware_path is not None and self.port is not None


# ---------------------------------------------------------------------------
class MyPanel1(wx.Panel):
    def __init__(self, parent):
        super(MyPanel1, self).__init__(parent)
        self._config = FlashConfig.getInstance()
        self._config = FlashConfig.load(self._get_config_file_path())
        self._init_ui()
        self._parent = parent

    def _init_ui(self):
        def on_baud_changed(event):
            radio_button = event.GetEventObject()
            if radio_button.GetValue():
                self._config.baud = radio_button.rate

        def on_mode_changed(event):
            radio_button = event.GetEventObject()
            if radio_button.GetValue():
                self._config.mode = radio_button.mode

        def on_erase_changed(event):
            radio_button = event.GetEventObject()
            if radio_button.GetValue():
                self._config.erase_before_flash = radio_button.erase

        def on_clicked(event):
            # self.console_ctrl.SetValue("")
            worker = FlashingThread(self, self._config)
            worker._panel2 = MyPanel2.GetInstance(self._parent)
            worker.start()

        def on_pick_file(event):
            self._config.firmware_path = event.GetPath().replace("'", "")

        hbox = wx.BoxSizer(wx.HORIZONTAL)
        fgs = wx.FlexGridSizer(4, 2, 10, 10)

        file_picker = wx.FilePickerCtrl(self, style=wx.FLP_USE_TEXTCTRL)
        file_picker.Bind(wx.EVT_FILEPICKER_CHANGED, on_pick_file)

        erase_boxsizer = wx.BoxSizer(wx.HORIZONTAL)

        def add_erase_radio_button(sizer, index, erase_before_flash, label, value):
            style = wx.RB_GROUP if index == 0 else 0
            radio_button = wx.RadioButton(
                self, name="erase-%s" % erase_before_flash, label="%s" % label, style=style)
            radio_button.Bind(wx.EVT_RADIOBUTTON, on_erase_changed)
            radio_button.erase = erase_before_flash
            radio_button.SetValue(value)
            sizer.Add(radio_button)
            sizer.AddSpacer(10)

        erase = self._config.erase_before_flash
        add_erase_radio_button(erase_boxsizer, 0, False, "no", erase is False)
        add_erase_radio_button(erase_boxsizer, 1, True,
                               "yes, wipes all data", erase is True)

        button = wx.Button(self, -1, "Flash Raiju")
        button.Bind(wx.EVT_BUTTON, on_clicked)

        file_label = wx.StaticText(self, label="Raiju firmware")
        erase_label = wx.StaticText(self, label="Erase flash")

        fgs.AddMany([
                    file_label, (file_picker, 1, wx.EXPAND),
                    erase_label, erase_boxsizer,
                    (wx.StaticText(self, label="")), (button, 1, wx.EXPAND),
                    ])
        fgs.AddGrowableCol(1, 1)
        hbox.Add(fgs, proportion=2, flag=wx.ALL | wx.EXPAND, border=15)
        self.SetSizer(hbox)

    @staticmethod
    def _get_config_file_path():
        return wx.StandardPaths.Get().GetUserConfigDir() + "/Raiju-tools.json"

# ---------------------------------------------------------------------------


class MyPanel2(wx.Panel):
    __instance = None

    @staticmethod
    def GetInstance(parent):
        if MyPanel2.__instance is None:
            MyPanel2.__instance = MyPanel2(parent)
        return MyPanel2.__instance

    def __init__(self, parent):
        if MyPanel2.__instance is not None:
            raise Exception(
                "MyPanel2 is a singleton class. Use MyPanel2.GetInstance() to get the instance.")
        super(MyPanel2, self).__init__(parent)
        self._init_ui()
        self._config = FlashConfig.getInstance()
        self._config = FlashConfig.load(self._get_config_file_path())
        self.ser = None  # Add an instance variable to hold the serial connection

    def _init_ui(self):

        self.timer = wx.Timer(self)
        self.Bind(wx.EVT_TIMER, self.time_interval, self.timer)

        fgs = wx.FlexGridSizer(2, 2, 10, 10)

        connectButton = wx.Button(self, -1, "Connect to Raiju")
        disconnectButton = wx.Button(self, -1, "Disconnect")
        connectButton.Bind(wx.EVT_BUTTON, self.on_clicked)
        disconnectButton.Bind(wx.EVT_BUTTON, self.on_disconnected)
        fgs.AddMany([(connectButton, 1, wx.EXPAND),
                    (disconnectButton, 1, wx.EXPAND)])
        hbox = wx.BoxSizer(wx.HORIZONTAL)

        hbox.Add(fgs, proportion=2, flag=wx.ALL | wx.EXPAND, border=15)

        self.SetSizer(hbox)

    def on_clicked(self, event):
        wx.CallAfter(self.start_timer)

    def start_timer(self):
        self.timer.Start(50)
        self.ser = serial.Serial(self._config.port, 115200, timeout=0.1)

    def on_disconnected(self, event):
        self.timer.Stop()
        if self.ser:
            self.ser.close()
            self.ser = None

    def time_interval(self, event):
        msg = self.ser.readline().decode().strip()
        if msg:
            print(msg)

    @staticmethod
    def _get_config_file_path():
        return wx.StandardPaths.Get().GetUserConfigDir() + "/raiju-tools.json"

# ---------------------------------------------------------------------------


class NodeMcuFlasher(wx.Frame):
    def __init__(self, parent, title):
        wx.Frame.__init__(self, parent, -1, title, size=(725, 650),
                          style=wx.DEFAULT_FRAME_STYLE | wx.NO_FULL_REPAINT_ON_RESIZE)
        self._config = FlashConfig.getInstance()
        self._config = FlashConfig.load(self._get_config_file_path())
        self._build_status_bar()
        self._set_icons()
        self._build_menu_bar()
        self._init_ui()

        sys.stdout = RedirectText(self.console_ctrl)
        print("Connect your device")
        print("")

    def _init_ui(self):
        def on_select_port(event):
            choice = event.GetEventObject()
            self._config.port = choice.GetString(choice.GetSelection())

        def on_baud_changed(event):
            radio_button = event.GetEventObject()

            if radio_button.GetValue():
                self._config.baud = radio_button.rate

        def on_reload(event):
            self.choice.SetItems(self._get_serial_ports())

        panel = wx.Panel(self)

        self.choice = wx.Choice(panel, choices=self._get_serial_ports())
        self.choice.SetSelection(0)
        self.choice.Bind(wx.EVT_CHOICE, on_select_port)
        self._select_configured_port()
        reload_button = wx.Button(panel, label="Reload")
        reload_button.Bind(wx.EVT_BUTTON, on_reload)
        reload_button.SetToolTip("Reload serial device list")

        serial_boxsizer = wx.BoxSizer(wx.HORIZONTAL)
        serial_boxsizer.Add(self.choice, 1, wx.EXPAND)
        serial_boxsizer.Add(reload_button, flag=wx.LEFT, border=10)
        baud_boxsizer = wx.BoxSizer(wx.HORIZONTAL)

        def add_baud_radio_button(sizer, index, baud_rate):
            style = wx.RB_GROUP if index == 0 else 0
            radio_button = wx.RadioButton(
                panel, name="baud-%d" % baud_rate, label="%d" % baud_rate, style=style)
            radio_button.rate = baud_rate
            # sets default value
            radio_button.SetValue(baud_rate == self._config.baud)
            radio_button.Bind(wx.EVT_RADIOBUTTON, on_baud_changed)
            sizer.Add(radio_button)
            sizer.AddSpacer(10)

        for idx, rate in enumerate(__supported_baud_rates__):
            add_baud_radio_button(baud_boxsizer, idx, rate)

        notebook = wx.Notebook(panel)
        notebook.AddPage(MyPanel1(notebook), "Flasher")
        notebook.AddPage(MyPanel2(notebook), "Monitor")

        self.console_ctrl = wx.TextCtrl(
            panel, style=wx.TE_MULTILINE | wx.TE_READONLY | wx.HSCROLL)
        self.console_ctrl.SetFont(wx.Font((0, 13), wx.FONTFAMILY_TELETYPE, wx.FONTSTYLE_NORMAL,
                                          wx.FONTWEIGHT_NORMAL))
        self.console_ctrl.SetBackgroundColour(wx.WHITE)
        self.console_ctrl.SetForegroundColour(wx.BLACK)
        self.console_ctrl.SetDefaultStyle(wx.TextAttr(wx.BLACK))

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(serial_boxsizer, 0, wx.ALL | wx.EXPAND, 5)
        sizer.Add(baud_boxsizer, 0, wx.ALL | wx.EXPAND, 5)
        sizer.Add(notebook, 0, wx.ALL | wx.EXPAND, 5)
        sizer.Add(self.console_ctrl, 1, wx.ALL | wx.EXPAND, 5)
        panel.SetSizer(sizer)
        self.Layout()
        self.Centre(wx.BOTH)
        self.Show()

    def _set_icons(self):
        self.SetIcon(images.Icon.GetIcon())

    def _build_status_bar(self):
        self.statusBar = self.CreateStatusBar(2, wx.STB_SIZEGRIP)
        self.statusBar.SetStatusWidths([-2, -1])
        status_text = "Raiju-tools %s" % __version__
        self.statusBar.SetStatusText(status_text, 0)

    def _build_menu_bar(self):
        self.menuBar = wx.MenuBar()

        # File menu
        file_menu = wx.Menu()
        wx.App.SetMacExitMenuItemId(wx.ID_EXIT)
        exit_item = file_menu.Append(
            wx.ID_EXIT, "E&xit\tCtrl-Q", "Exit Raiju-tools")
        self.Bind(wx.EVT_MENU, self._on_exit_app, exit_item)
        self.menuBar.Append(file_menu, "&File")

        # Help menu
        help_menu = wx.Menu()
        help_item = help_menu.Append(
            wx.ID_ABOUT, '&About Raiju-tools', 'About')
        self.Bind(wx.EVT_MENU, self._on_help_about, help_item)
        self.menuBar.Append(help_menu, '&Help')

        self.SetMenuBar(self.menuBar)

    # Menu methods
    def _on_exit_app(self, event):
        self._config.safe(self._get_config_file_path())
        self.Close(True)

    def _on_help_about(self, event):
        from About import AboutDlg
        about = AboutDlg(self)
        about.ShowModal()
        about.Destroy()

    def report_error(self, message):
        self.console_ctrl.SetValue(message)

    def log_message(self, message):
        self.console_ctrl.AppendText(message)

# SERIAL SELECTOR
    def _select_configured_port(self):
        count = 0
        for item in self.choice.GetItems():
            if item == self._config.port:
                self.choice.Select(count)
                break
            count += 1

    @staticmethod
    def _get_serial_ports():
        ports = [__auto_select__]
        for port, desc, hwid in sorted(list_ports.comports()):
            ports.append(port)
        return ports

    @staticmethod
    def _get_config_file_path():
        return wx.StandardPaths.Get().GetUserConfigDir() + "/Raiju-tools.json"

# ---------------------------------------------------------------------------


# ---------------------------------------------------------------------------
class MySplashScreen(wx.adv.SplashScreen):
    def __init__(self):
        wx.adv.SplashScreen.__init__(self, images.Splash.GetBitmap(),
                                     wx.adv.SPLASH_CENTRE_ON_SCREEN | wx.adv.SPLASH_TIMEOUT, 2000, None, -1)
        self.Bind(wx.EVT_CLOSE, self._on_close)
        self.__fc = wx.CallLater(2000, self._show_main)

    def _on_close(self, evt):
        # Make sure the default handler runs too so this window gets
        # destroyed
        evt.Skip()
        self.Hide()

        # if the timer is still running then go ahead and show the
        # main frame now
        if self.__fc.IsRunning():
            self.__fc.Stop()
            self._show_main()

    def _show_main(self):
        frame = NodeMcuFlasher(None, "Raiju-tools")
        frame.Show()
        if self.__fc.IsRunning():
            self.Raise()

# ---------------------------------------------------------------------------


# ----------------------------------------------------------------------------
class App(wx.App, wx.lib.mixins.inspection.InspectionMixin):
    def OnInit(self):
        # see https://discuss.wxpython.org/t/wxpython4-1-1-python3-8-locale-wxassertionerror/35168
        self.ResetLocale()
        wx.SystemOptions.SetOption("mac.window-plain-transition", 1)
        self.SetAppName("Raiju-tools")

        # Create and show the splash screen.  It will then create and
        # show the main frame when it is time to do so.  Normally when
        # using a SplashScreen you would create it, show it and then
        # continue on with the application's initialization, finally
        # creating and showing the main application window(s).  In
        # this case we have nothing else to do so we'll delay showing
        # the main frame until later (see ShowMain above) so the users
        # can see the SplashScreen effect.
        splash = MySplashScreen()
        splash.Show()

        return True


# ---------------------------------------------------------------------------
def main():
    app = App(False)
    app.MainLoop()
# ---------------------------------------------------------------------------


if __name__ == '__main__':
    __name__ = 'Main'
    main()
