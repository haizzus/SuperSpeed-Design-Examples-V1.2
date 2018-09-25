// ***********************************************************************
// Assembly         : USBBarometer
// Author           : Bob Beauchaine
// Created          : 06-10-2014
//
// Last Modified By : Bob
// Last Modified On : 07-07-2014
// ***********************************************************************
// <copyright file="Form1.cs" company="">
//     Copyright (c) . All rights reserved.
// </copyright>
// <summary></summary>
// ***********************************************************************
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Pipes;
using CyUSB;
using Engine;

/// <summary>
/// The USBBarometer namespace.
/// </summary>
namespace USBBarometer
{
    /// <summary>
    /// Class Form1.
    /// </summary>
    public partial class Form1 : Form
    {
        /// <summary>
        /// The Cypress Vendor ID.  Does not change
        /// </summary>
        private int VENDOR_ID = 0x04B4;
        /// <summary>
        /// The Cypress product ID.  This is typical, but 0x00F0 is used as well
        /// </summary>
        private int PRODUCT_ID = 0x00F1;
        /// <summary>
        /// The USB engine.  Does all of the heavy lifting
        /// </summary>
        private USBEngine theEngine = null;
        /// <summary>
        /// The usb device
        /// </summary>
        private CyUSBDevice theUSBDevice = null;
        /// <summary>
        /// The list of all available Cypress USB devices.  Usually only one is ever present
        /// </summary>
        private USBDeviceList usbDevices = null;
        /// <summary>
        /// The Bulk in endpoint object
        /// </summary>
        private CyBulkEndPoint inEndpoint = null;
        /// <summary>
        /// The Bulk out endpoint
        /// </summary>
        private CyBulkEndPoint outEndpoint = null;
        /// <summary>
        /// The throughput values
        /// </summary>
        private List<long> throughputValues = new List<long>();

        /// <summary>
        /// Initializes a new instance of the <see cref="Form1"/> class.
        /// </summary>
        public Form1()
        {
            InitializeComponent();

            // Create a list of CYUSB devices
            usbDevices = new USBDeviceList(CyConst.DEVICES_CYUSB);

            //Adding event handlers for device attachment and device removal
            usbDevices.DeviceAttached += new EventHandler(USBDeviceAttached);
            usbDevices.DeviceRemoved += new EventHandler(USBDeviceRemoved);
            
            theEngine = new USBEngine();
            theEngine.factoryEvent += ThroughputCallback;

            PPXComboBox.SelectedIndex = InterleavedComboBox.SelectedIndex = 3;
            FileBlockSizeComboBox.SelectedIndex = 4;

            UpdateUSBDevice();

            SourceListBox.SelectedItem = "Memory";
            SinkListBox.SelectedItem = "Memory";
        }

        /// <summary>
        /// Updates the usb device.  Registered as a callback against the device
        /// driver, which will notify us (usually) when a new USB peripheral is 
        /// attached to the driver via Windows
        /// </summary>
        public void UpdateUSBDevice()
        {
            theUSBDevice = usbDevices[VENDOR_ID, PRODUCT_ID] as CyUSBDevice;

            if (theUSBDevice == null)
            {
                PRODUCT_ID = 0x00F0;
                theUSBDevice = usbDevices[VENDOR_ID, PRODUCT_ID] as CyUSBDevice;
            }

            if (theUSBDevice != null)
            {
                USBDeviceNameLabel.Text = theUSBDevice.FriendlyName;

                outEndpoint = theUSBDevice.EndPointOf(0x01) as CyBulkEndPoint;
                inEndpoint = theUSBDevice.EndPointOf(0x81) as CyBulkEndPoint;

                if ((outEndpoint != null) && (inEndpoint != null))
                {
                    //make sure that the device configuration doesn't contain the other than bulk endpoint
                    if ((outEndpoint.Attributes & 0x03) != 0x02)
                        USBOutEndpointLabel.Text = "<incorrect endpoint type detected - expected a bulk endpoint>";
                    else
                        USBOutEndpointLabel.Text = "Bulk out endpoint 0x01";

                    if ((inEndpoint.Attributes & 0x03) != 0x02)
                        USBInEndpointLabel.Text = "<incorrect endpoint type detected - expected a bulk endpoint>";
                    else
                        USBInEndpointLabel.Text = "Bulk in endpoint 0x81";

                    outEndpoint.TimeOut = 1000;
                    inEndpoint.TimeOut = 1000;
                }
            }
            else
            {
                USBDeviceNameLabel.Text = USBOutEndpointLabel.Text = USBInEndpointLabel.Text = "<no device detected>";
            }
        }

        /// <summary>
        /// Callback when a Cypress USB device is removed.
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
        void USBDeviceRemoved(object sender, EventArgs e)
        {
            UpdateUSBDevice();
        }


        /// <summary>
        /// Callback when a new Cypress device is attached.
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
        void USBDeviceAttached(object sender, EventArgs e)
        {
            UpdateUSBDevice();
        }

        /// <summary>
        /// Callback for throughput messages from the engine.  Used to update the gauge and graphs
        /// </summary>
        /// <param name="consumedUnits">How many megabytes in total have been processed by the engine since it was started.</param>
        private void ThroughputCallback(long consumedUnits)
        {
            // Since this is called on a separate thread, use the invoker to talk to the 
            // GUI controls
            Invoke( ( MethodInvoker ) delegate {
                ProductionTextBox.Text = consumedUnits.ToString();
                TotalBytesTextBox.Text = (Convert.ToInt64(TotalBytesTextBox.Text) + consumedUnits).ToString();
                throughputValues.Add(consumedUnits);

                // Auto-scale the gauge if this value is larger than the currently tracked max
                while(ThroughputGauge.MaxValue < consumedUnits)
                {
                    ThroughputGauge.MaxValue += 100;
                }
                // Autoscale the major ticks.  Allow at most 10 major ticks over the entire gauge range
                while (ThroughputGauge.MaxValue / ThroughputGauge.ScaleLinesMajorStepValue > 10)
                    ThroughputGauge.ScaleLinesMajorStepValue += 100;

                ThroughputGauge.Value = consumedUnits;

                // Add this data point to the chart
                ThroughputChart.Series[0].Points.AddY(consumedUnits);
                int count = ThroughputChart.Series[0].Points.Count;
                // Truncate the chart at the last 30 seconds
                if(ThroughputChart.Series[0].Points.Count > 30)
                    ThroughputChart.Series[0].Points.RemoveAt(0);
                ThroughputChart.ChartAreas[0].RecalculateAxesScale();
            });
        }

        /// <summary>
        /// Resets the throughput metrics.
        /// </summary>
        void ResetThroughputMetrics()
        {
            ProductionTextBox.Text = TotalBytesTextBox.Text = "0";
            ThroughputGauge.MaxValue = 400;
            ThroughputGauge.ScaleLinesMajorStepValue = 100;
            ThroughputGauge.Value = 0;
            ThroughputChart.Series[0].Points.Clear();
            ThroughputChart.ChartAreas[0].RecalculateAxesScale();
            throughputValues.Clear();
        }

        /// <summary>
        /// Validates user selections for selected producer/consumer selected
        /// </summary>
        /// <returns><c>true</c> if the user has selected values for all mandatory items for this mode, <c>false</c> otherwise.</returns>
        private bool ValidateForSelectedMode()
        {
            bool retVal = true;

            // Need a source file for File sources
            if (SourceListBox.SelectedItem.ToString() == "File")
                if (openFileDialog1.FileName == openFileDialog1.SafeFileName)
                {
                    MessageBox.Show("Please select a source file to stream");
                    retVal = false;
                }

            // Need a sink file for File sinks
            if (SinkListBox.SelectedItem.ToString() == "File")
                if (saveFileDialog1.FileName == "")
                {
                    MessageBox.Show("Please select a destination file");
                    retVal = false;
                }

            return retVal;
        }

        /// <summary>
        /// Creates the duplex link.  Used only in the USB -> USB mode
        /// </summary>
        private void CreateDuplexLink()
        {
            // Create two links, one to feed the USB and the other to read the USB echo
            theEngine.DownloadPipe = new SourceSinkPair();
            theEngine.UploadPipe = new SourceSinkPair();

            // Create two new sources and sinks
            theEngine.DownloadPipe.consumer = new Sink();
            theEngine.DownloadPipe.producer = new Source();
            theEngine.UploadPipe.producer = new Source();
            theEngine.UploadPipe.consumer = new Sink();

            // Set aside the circular buffers
            theEngine.DownloadPipe.bufferSize = theEngine.UploadPipe.bufferSize = 250 * 1024 * 1024;
            theEngine.DownloadPipe.producer.bytesPerWrite = theEngine.DownloadPipe.consumer.bytesPerRead = Convert.ToInt32(FileBlockSizeComboBox.SelectedItem) * 1024;
            theEngine.DownloadPipe.producer.producer = ProducerTypes.USBProducer;
            theEngine.DownloadPipe.consumer.consumer = ConsumerTypes.MemoryConsumer;

            theEngine.UploadPipe.bufferSize = theEngine.UploadPipe.bufferSize = 250 * 1024 * 1024;
            theEngine.UploadPipe.producer.producer = ProducerTypes.MemoryProducer;
            theEngine.UploadPipe.producer.bytesPerWrite = theEngine.UploadPipe.consumer.bytesPerRead = Convert.ToInt32(FileBlockSizeComboBox.SelectedItem) * 1024;
            theEngine.UploadPipe.consumer.consumer = ConsumerTypes.USBConsumer;
        }

        /// <summary>
        /// Creates the simplex link.  Used for all modes except USB->USB
        /// </summary>
        private void CreateSimplexLink()
        {
            theEngine.DownloadPipe = new SourceSinkPair();
            theEngine.DownloadPipe.bufferSize = 250 * 1024 * 1024;
            
            theEngine.DownloadPipe.producer = new Source();
            theEngine.DownloadPipe.producer.bytesPerWrite = Convert.ToInt32(FileBlockSizeComboBox.SelectedItem) * 1024;
            theEngine.DownloadPipe.producer.producer = (ProducerTypes)SourceListBox.SelectedIndex;
            theEngine.DownloadPipe.producer.fileName = openFileDialog1.FileName;

            theEngine.DownloadPipe.consumer = new Sink();
            theEngine.DownloadPipe.consumer.consumer = (ConsumerTypes)SinkListBox.SelectedIndex;
            theEngine.DownloadPipe.consumer.bytesPerRead = Convert.ToInt32(FileBlockSizeComboBox.SelectedItem) * 1024;
            theEngine.DownloadPipe.consumer.fileName = saveFileDialog1.FileName;
        }

        /// <summary>
        /// Configures the USB engine with user provided entries.
        /// </summary>
        private void ConfigureEngine()
        {
            theEngine.LinkParameters = new USBParameters();
            theEngine.LinkParameters.VendorID = VENDOR_ID;
            theEngine.LinkParameters.ProductID = PRODUCT_ID;
            theEngine.LinkParameters.PacketsPerTransfer = Convert.ToInt32(PPXComboBox.SelectedItem);
            theEngine.LinkParameters.ParallelTransfers = Convert.ToInt32(InterleavedComboBox.SelectedItem);

            if ((SourceListBox.SelectedItem.ToString() == "USB 3.0") && (SinkListBox.SelectedItem.ToString() == "USB 3.0"))
                CreateDuplexLink();
            else
                CreateSimplexLink();
        }

        /// <summary>
        /// Handles the Click event of the GoButton control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
        private void GoButton_Click(object sender, EventArgs e)
        {

            if (ValidateForSelectedMode())
            {
                ResetThroughputMetrics();
                ConfigureEngine();

                GoButton.Enabled = CharacterizeButton.Enabled = false;
                StopButton.Enabled = true;
                theEngine.Run();
            }
        }

        /// <summary>
        /// Handles the Click event of the StopButton control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
        private void StopButton_Click(object sender, EventArgs e)
        {
            theEngine.Stop();
            GoButton.Enabled = CharacterizeButton.Enabled = true;
            StopButton.Enabled = false;
        }

        /// <summary>
        /// Handles the Click event of the SourceFileButton control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
        private void SourceFileButton_Click(object sender, EventArgs e)
        {
            openFileDialog1.ShowDialog();
        }

        /// <summary>
        /// Handles the Click event of the DestinationFileButton control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
        private void DestinationFileButton_Click(object sender, EventArgs e)
        {
            saveFileDialog1.ShowDialog();
        }

        /// <summary>
        /// Handles the SelectedIndexChanged event of the SinkListBox control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
        private void SinkListBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            SinkPictureBox.Image = imageList1.Images[SinkListBox.SelectedIndex]; 
        }

        /// <summary>
        /// Handles the SelectedIndexChanged event of the SoureListBox control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
        private void SoureListBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            SourcePictureBox.Image = imageList1.Images[SourceListBox.SelectedIndex]; 
        }

        /// <summary>
        /// Handles the FormClosing event of the Form1 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="FormClosingEventArgs"/> instance containing the event data.</param>
        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            theEngine.Stop();
        }



        /// <summary>
        /// Characterizations the thread.
        /// </summary>
        private void CharacterizationThread()
        {

            var permutations = new List<Tuple<ProducerTypes, ConsumerTypes>>();
            permutations.Add(new Tuple<ProducerTypes, ConsumerTypes>(ProducerTypes.USBProducer, ConsumerTypes.MemoryConsumer));
            permutations.Add(new Tuple<ProducerTypes, ConsumerTypes>(ProducerTypes.MemoryProducer, ConsumerTypes.USBConsumer));
            permutations.Add(new Tuple<ProducerTypes, ConsumerTypes>(ProducerTypes.DiskProducer, ConsumerTypes.USBConsumer));
            permutations.Add(new Tuple<ProducerTypes, ConsumerTypes>(ProducerTypes.USBProducer, ConsumerTypes.DiskConsumer));
            permutations.Add(new Tuple<ProducerTypes, ConsumerTypes>(ProducerTypes.USBProducer, ConsumerTypes.USBConsumer));

            String[] strings = { "USB 3.0 -> Host Memory", "Host Memory -> USB 3.0", "Disk file -> USB 3.0", "USB 3.0 -> Disk file", "USB 3.0 -> USB 3.0" };
            var results = new List<Tuple<String, double>>();

            int index = 0;
            foreach (var configuration in permutations)
            {

                Invoke((MethodInvoker)delegate
                {
                    SourceListBox.SelectedIndex = (int)configuration.Item1;
                    SinkListBox.SelectedIndex = (int)configuration.Item2;
                    ResetThroughputMetrics();
                    ConfigureEngine();
                });

                theEngine.Run();
                System.Threading.Thread.Sleep(30000);
                theEngine.Stop();

                Invoke((MethodInvoker)delegate
                {
                    double sum = 0;
                    for (int i = 1; i <= 5; ++i)
                        sum += throughputValues[throughputValues.Count() - i];

                    sum /= 5;
                    results.Add(new Tuple<String, double>(strings[index], sum));
                });

                index++;
            }

            Invoke((MethodInvoker)delegate
            {
                CharacterizeButton.Enabled = GoButton.Enabled = StopButton.Enabled = ParametersGroupBox.Enabled = LinkDefinitionGroupBox.Enabled = true;
            });


            var summary = new SummaryForm(results);
            summary.ShowDialog();
        }

        /// <summary>
        /// Handles the Click event of the button3 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
        private void button3_Click(object sender, EventArgs e)
        {
            if (openFileDialog1.FileName == openFileDialog1.SafeFileName)
            {
                MessageBox.Show("Please select a source file to stream");
            }
            else if (saveFileDialog1.FileName == "")
            {
                MessageBox.Show("Please select a destination file");
            }
            else
            {
                CharacterizeButton.Enabled = GoButton.Enabled = StopButton.Enabled = ParametersGroupBox.Enabled = LinkDefinitionGroupBox.Enabled = false;
               
                var characterizationThread = new System.Threading.Thread(CharacterizationThread);
                characterizationThread.Start();
            }
        }
    }
}

