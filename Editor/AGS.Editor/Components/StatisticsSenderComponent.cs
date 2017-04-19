using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net;
using System.Text;
using System.Threading;

namespace AGS.Editor.Components
{
    internal class StatisticsSenderComponent : BaseComponent
    {
        private const string STATS_REPORT_URL = @"http://www.adventuregamestudio.co.uk/clientstats.php";
        private const int STATS_SEND_INTERVAL_IN_DAYS = 30;
        private Timer _timer;
        private int _screenWidth;
        private int _screenHeight;

        public StatisticsSenderComponent(GUIController guiController, AGSEditor agsEditor)
            : base(guiController, agsEditor)
        {
            // Screen width x height must be cached here, otherwise we could attempt to
            // retrieve them while the game is being tested full-screen
            _screenWidth = System.Windows.Forms.SystemInformation.PrimaryMonitorSize.Width;
            _screenHeight = System.Windows.Forms.SystemInformation.PrimaryMonitorSize.Height;
            _timer = new Timer(new TimerCallback(timer_Callback), null, 120000, 240000);
        }

        public override string ComponentID
        {
            get { return ComponentIDs.StatisticsSender; }
        }

        private void timer_Callback(object parameter)
        {
            try
            {
                if ((!_agsEditor.Preferences.SendAnonymousStats) ||
                    (DateTime.Now.Subtract(_agsEditor.Preferences.StatsLastSent).TotalDays < STATS_SEND_INTERVAL_IN_DAYS))
                {
                    return;
                }
                string osVersion = Environment.OSVersion.VersionString;
                string netVersion = Utilities.NetRuntimeVersion;
                string agsVersion = AGS.Types.Version.AGS_EDITOR_VERSION;
                string resolution = _screenWidth.ToString() + "x" + _screenHeight.ToString();

                string queryString = string.Format(
                    "action=sendinfo&winver={0}&dotnetver={1}&agsver={2}&resolution={3}",
                    osVersion, netVersion, agsVersion, resolution);

                WebClient client = new WebClient();
                client.Headers.Add(HttpRequestHeader.ContentType, "application/x-www-form-urlencoded");
                string response = client.UploadString(STATS_REPORT_URL, queryString);
                if (response.Contains("200 Received"))
                {
                    _agsEditor.Preferences.StatsLastSent = DateTime.Now;
                    _agsEditor.Preferences.SaveToRegistry();
                }
            }
            catch (Exception)
            {
                // problems here should never interfere with the user
            }
        }
    }
}
