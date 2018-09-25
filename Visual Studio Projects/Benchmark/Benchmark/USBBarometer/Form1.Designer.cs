namespace USBBarometer
{
    partial class Form1
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
            System.Windows.Forms.DataVisualization.Charting.ChartArea chartArea1 = new System.Windows.Forms.DataVisualization.Charting.ChartArea();
            System.Windows.Forms.DataVisualization.Charting.Series series1 = new System.Windows.Forms.DataVisualization.Charting.Series();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.ThroughputGauge = new System.Windows.Forms.AGauge();
            this.ThroughputChart = new System.Windows.Forms.DataVisualization.Charting.Chart();
            this.GoButton = new System.Windows.Forms.Button();
            this.StopButton = new System.Windows.Forms.Button();
            this.ProductionTextBox = new System.Windows.Forms.TextBox();
            this.ParametersGroupBox = new System.Windows.Forms.GroupBox();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.PPXComboBox = new System.Windows.Forms.ComboBox();
            this.InterleavedComboBox = new System.Windows.Forms.ComboBox();
            this.label3 = new System.Windows.Forms.Label();
            this.FileBlockSizeComboBox = new System.Windows.Forms.ComboBox();
            this.label10 = new System.Windows.Forms.Label();
            this.label11 = new System.Windows.Forms.Label();
            this.SourceFileButton = new System.Windows.Forms.Button();
            this.DestinationFileButton = new System.Windows.Forms.Button();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.USBDeviceNameLabel = new System.Windows.Forms.Label();
            this.USBInEndpointLabel = new System.Windows.Forms.Label();
            this.USBOutEndpointLabel = new System.Windows.Forms.Label();
            this.SourceListBox = new System.Windows.Forms.ListBox();
            this.SinkListBox = new System.Windows.Forms.ListBox();
            this.TotalBytesTextBox = new System.Windows.Forms.TextBox();
            this.label7 = new System.Windows.Forms.Label();
            this.SourcePictureBox = new System.Windows.Forms.PictureBox();
            this.SinkPictureBox = new System.Windows.Forms.PictureBox();
            this.pictureBox6 = new System.Windows.Forms.PictureBox();
            this.LinkDefinitionGroupBox = new System.Windows.Forms.GroupBox();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.saveFileDialog1 = new System.Windows.Forms.SaveFileDialog();
            this.label8 = new System.Windows.Forms.Label();
            this.imageList1 = new System.Windows.Forms.ImageList(this.components);
            this.CharacterizeButton = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.ThroughputChart)).BeginInit();
            this.ParametersGroupBox.SuspendLayout();
            this.tableLayoutPanel1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.tableLayoutPanel2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.SourcePictureBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.SinkPictureBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox6)).BeginInit();
            this.LinkDefinitionGroupBox.SuspendLayout();
            this.SuspendLayout();
            // 
            // ThroughputGauge
            // 
            this.ThroughputGauge.BaseArcColor = System.Drawing.Color.Gray;
            this.ThroughputGauge.BaseArcRadius = 80;
            this.ThroughputGauge.BaseArcStart = 135;
            this.ThroughputGauge.BaseArcSweep = 270;
            this.ThroughputGauge.BaseArcWidth = 2;
            this.ThroughputGauge.Center = new System.Drawing.Point(150, 100);
            this.ThroughputGauge.Location = new System.Drawing.Point(12, 233);
            this.ThroughputGauge.MaxValue = 400F;
            this.ThroughputGauge.MinValue = 0F;
            this.ThroughputGauge.Name = "ThroughputGauge";
            this.ThroughputGauge.NeedleColor1 = System.Windows.Forms.AGaugeNeedleColor.Gray;
            this.ThroughputGauge.NeedleColor2 = System.Drawing.Color.DimGray;
            this.ThroughputGauge.NeedleRadius = 80;
            this.ThroughputGauge.NeedleType = System.Windows.Forms.NeedleType.Advance;
            this.ThroughputGauge.NeedleWidth = 2;
            this.ThroughputGauge.ScaleLinesInterColor = System.Drawing.Color.Black;
            this.ThroughputGauge.ScaleLinesInterInnerRadius = 73;
            this.ThroughputGauge.ScaleLinesInterOuterRadius = 80;
            this.ThroughputGauge.ScaleLinesInterWidth = 1;
            this.ThroughputGauge.ScaleLinesMajorColor = System.Drawing.Color.Black;
            this.ThroughputGauge.ScaleLinesMajorInnerRadius = 70;
            this.ThroughputGauge.ScaleLinesMajorOuterRadius = 80;
            this.ThroughputGauge.ScaleLinesMajorStepValue = 100F;
            this.ThroughputGauge.ScaleLinesMajorWidth = 2;
            this.ThroughputGauge.ScaleLinesMinorColor = System.Drawing.Color.Gray;
            this.ThroughputGauge.ScaleLinesMinorInnerRadius = 75;
            this.ThroughputGauge.ScaleLinesMinorOuterRadius = 80;
            this.ThroughputGauge.ScaleLinesMinorTicks = 9;
            this.ThroughputGauge.ScaleLinesMinorWidth = 1;
            this.ThroughputGauge.ScaleNumbersColor = System.Drawing.Color.Black;
            this.ThroughputGauge.ScaleNumbersFormat = null;
            this.ThroughputGauge.ScaleNumbersRadius = 95;
            this.ThroughputGauge.ScaleNumbersRotation = 0;
            this.ThroughputGauge.ScaleNumbersStartScaleLine = 0;
            this.ThroughputGauge.ScaleNumbersStepScaleLines = 1;
            this.ThroughputGauge.Size = new System.Drawing.Size(300, 180);
            this.ThroughputGauge.TabIndex = 0;
            this.ThroughputGauge.Text = "aGauge1";
            this.ThroughputGauge.Value = 0F;
            // 
            // ThroughputChart
            // 
            chartArea1.AxisX.MajorGrid.Enabled = false;
            chartArea1.AxisY.MajorGrid.Enabled = false;
            chartArea1.Name = "ChartArea1";
            this.ThroughputChart.ChartAreas.Add(chartArea1);
            this.ThroughputChart.Location = new System.Drawing.Point(285, 260);
            this.ThroughputChart.Name = "ThroughputChart";
            this.ThroughputChart.Palette = System.Windows.Forms.DataVisualization.Charting.ChartColorPalette.Bright;
            series1.ChartArea = "ChartArea1";
            series1.ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.StepLine;
            series1.Name = "Series1";
            this.ThroughputChart.Series.Add(series1);
            this.ThroughputChart.Size = new System.Drawing.Size(523, 134);
            this.ThroughputChart.TabIndex = 2;
            this.ThroughputChart.Text = "chart1";
            // 
            // GoButton
            // 
            this.GoButton.Location = new System.Drawing.Point(805, 131);
            this.GoButton.Name = "GoButton";
            this.GoButton.Size = new System.Drawing.Size(75, 23);
            this.GoButton.TabIndex = 4;
            this.GoButton.Text = "Go";
            this.GoButton.UseVisualStyleBackColor = true;
            this.GoButton.Click += new System.EventHandler(this.GoButton_Click);
            // 
            // StopButton
            // 
            this.StopButton.Enabled = false;
            this.StopButton.Location = new System.Drawing.Point(805, 160);
            this.StopButton.Name = "StopButton";
            this.StopButton.Size = new System.Drawing.Size(75, 23);
            this.StopButton.TabIndex = 5;
            this.StopButton.Text = "Stop";
            this.StopButton.UseVisualStyleBackColor = true;
            this.StopButton.Click += new System.EventHandler(this.StopButton_Click);
            // 
            // ProductionTextBox
            // 
            this.ProductionTextBox.Location = new System.Drawing.Point(134, 393);
            this.ProductionTextBox.Name = "ProductionTextBox";
            this.ProductionTextBox.ReadOnly = true;
            this.ProductionTextBox.Size = new System.Drawing.Size(59, 20);
            this.ProductionTextBox.TabIndex = 6;
            this.ProductionTextBox.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // ParametersGroupBox
            // 
            this.ParametersGroupBox.Controls.Add(this.tableLayoutPanel1);
            this.ParametersGroupBox.Location = new System.Drawing.Point(482, 19);
            this.ParametersGroupBox.Name = "ParametersGroupBox";
            this.ParametersGroupBox.Size = new System.Drawing.Size(303, 210);
            this.ParametersGroupBox.TabIndex = 8;
            this.ParametersGroupBox.TabStop = false;
            this.ParametersGroupBox.Text = "Parameters";
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.tableLayoutPanel1.ColumnCount = 2;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel1.Controls.Add(this.label1, 0, 0);
            this.tableLayoutPanel1.Controls.Add(this.label2, 0, 1);
            this.tableLayoutPanel1.Controls.Add(this.PPXComboBox, 1, 0);
            this.tableLayoutPanel1.Controls.Add(this.InterleavedComboBox, 1, 1);
            this.tableLayoutPanel1.Controls.Add(this.label3, 0, 2);
            this.tableLayoutPanel1.Controls.Add(this.FileBlockSizeComboBox, 1, 2);
            this.tableLayoutPanel1.Controls.Add(this.label10, 0, 4);
            this.tableLayoutPanel1.Controls.Add(this.label11, 0, 5);
            this.tableLayoutPanel1.Controls.Add(this.SourceFileButton, 1, 4);
            this.tableLayoutPanel1.Controls.Add(this.DestinationFileButton, 1, 5);
            this.tableLayoutPanel1.Location = new System.Drawing.Point(24, 31);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 6;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 28F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 19F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(260, 138);
            this.tableLayoutPanel1.TabIndex = 0;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.label1.Location = new System.Drawing.Point(3, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(158, 27);
            this.label1.TabIndex = 0;
            this.label1.Text = "USB Packets per transfer";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.label2.Location = new System.Drawing.Point(3, 27);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(158, 27);
            this.label2.TabIndex = 1;
            this.label2.Text = "USB Interleaved transfers";
            this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // PPXComboBox
            // 
            this.PPXComboBox.Dock = System.Windows.Forms.DockStyle.Fill;
            this.PPXComboBox.FormattingEnabled = true;
            this.PPXComboBox.Items.AddRange(new object[] {
            "1",
            "2",
            "4",
            "8",
            "16",
            "32",
            "64"});
            this.PPXComboBox.Location = new System.Drawing.Point(167, 3);
            this.PPXComboBox.Name = "PPXComboBox";
            this.PPXComboBox.Size = new System.Drawing.Size(90, 21);
            this.PPXComboBox.TabIndex = 2;
            // 
            // InterleavedComboBox
            // 
            this.InterleavedComboBox.Dock = System.Windows.Forms.DockStyle.Fill;
            this.InterleavedComboBox.FormattingEnabled = true;
            this.InterleavedComboBox.Items.AddRange(new object[] {
            "1",
            "2",
            "4",
            "8",
            "16",
            "32",
            "64",
            "128"});
            this.InterleavedComboBox.Location = new System.Drawing.Point(167, 30);
            this.InterleavedComboBox.Name = "InterleavedComboBox";
            this.InterleavedComboBox.Size = new System.Drawing.Size(90, 21);
            this.InterleavedComboBox.TabIndex = 3;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Dock = System.Windows.Forms.DockStyle.Fill;
            this.label3.Location = new System.Drawing.Point(3, 54);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(158, 27);
            this.label3.TabIndex = 4;
            this.label3.Text = "Buffer read/write block size (kB)";
            this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // FileBlockSizeComboBox
            // 
            this.FileBlockSizeComboBox.Dock = System.Windows.Forms.DockStyle.Fill;
            this.FileBlockSizeComboBox.FormattingEnabled = true;
            this.FileBlockSizeComboBox.Items.AddRange(new object[] {
            "64",
            "128",
            "256",
            "512",
            "1024",
            "2048",
            "4096",
            "8192"});
            this.FileBlockSizeComboBox.Location = new System.Drawing.Point(167, 57);
            this.FileBlockSizeComboBox.Name = "FileBlockSizeComboBox";
            this.FileBlockSizeComboBox.Size = new System.Drawing.Size(90, 21);
            this.FileBlockSizeComboBox.TabIndex = 5;
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Dock = System.Windows.Forms.DockStyle.Fill;
            this.label10.Location = new System.Drawing.Point(3, 81);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(158, 28);
            this.label10.TabIndex = 8;
            this.label10.Text = "Choose source file";
            this.label10.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Dock = System.Windows.Forms.DockStyle.Fill;
            this.label11.Location = new System.Drawing.Point(3, 109);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(158, 29);
            this.label11.TabIndex = 9;
            this.label11.Text = "Choose destination file";
            this.label11.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // SourceFileButton
            // 
            this.SourceFileButton.Dock = System.Windows.Forms.DockStyle.Fill;
            this.SourceFileButton.Location = new System.Drawing.Point(167, 84);
            this.SourceFileButton.Name = "SourceFileButton";
            this.SourceFileButton.Size = new System.Drawing.Size(90, 22);
            this.SourceFileButton.TabIndex = 10;
            this.SourceFileButton.Text = "Choose...";
            this.SourceFileButton.UseVisualStyleBackColor = true;
            this.SourceFileButton.Click += new System.EventHandler(this.SourceFileButton_Click);
            // 
            // DestinationFileButton
            // 
            this.DestinationFileButton.Dock = System.Windows.Forms.DockStyle.Fill;
            this.DestinationFileButton.Location = new System.Drawing.Point(167, 112);
            this.DestinationFileButton.Name = "DestinationFileButton";
            this.DestinationFileButton.Size = new System.Drawing.Size(90, 23);
            this.DestinationFileButton.TabIndex = 11;
            this.DestinationFileButton.Text = "Choose...";
            this.DestinationFileButton.UseVisualStyleBackColor = true;
            this.DestinationFileButton.Click += new System.EventHandler(this.DestinationFileButton_Click);
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.tableLayoutPanel2);
            this.groupBox2.Location = new System.Drawing.Point(12, 12);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(282, 85);
            this.groupBox2.TabIndex = 9;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "USB";
            // 
            // tableLayoutPanel2
            // 
            this.tableLayoutPanel2.ColumnCount = 2;
            this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel2.Controls.Add(this.label4, 0, 0);
            this.tableLayoutPanel2.Controls.Add(this.label5, 0, 1);
            this.tableLayoutPanel2.Controls.Add(this.label6, 0, 2);
            this.tableLayoutPanel2.Controls.Add(this.USBDeviceNameLabel, 1, 0);
            this.tableLayoutPanel2.Controls.Add(this.USBInEndpointLabel, 1, 1);
            this.tableLayoutPanel2.Controls.Add(this.USBOutEndpointLabel, 1, 2);
            this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel2.Location = new System.Drawing.Point(3, 16);
            this.tableLayoutPanel2.Name = "tableLayoutPanel2";
            this.tableLayoutPanel2.RowCount = 3;
            this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
            this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
            this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
            this.tableLayoutPanel2.Size = new System.Drawing.Size(276, 66);
            this.tableLayoutPanel2.TabIndex = 0;
            // 
            // label4
            // 
            this.label4.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(3, 0);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(78, 22);
            this.label4.TabIndex = 0;
            this.label4.Text = "Device:";
            this.label4.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label5
            // 
            this.label5.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(3, 22);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(78, 22);
            this.label5.TabIndex = 1;
            this.label5.Text = "IN Endpoint:";
            this.label5.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label6
            // 
            this.label6.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(3, 44);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(78, 22);
            this.label6.TabIndex = 2;
            this.label6.Text = "OUT Endpoint:";
            this.label6.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // USBDeviceNameLabel
            // 
            this.USBDeviceNameLabel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.USBDeviceNameLabel.AutoSize = true;
            this.USBDeviceNameLabel.Location = new System.Drawing.Point(87, 0);
            this.USBDeviceNameLabel.Name = "USBDeviceNameLabel";
            this.USBDeviceNameLabel.Size = new System.Drawing.Size(186, 22);
            this.USBDeviceNameLabel.TabIndex = 3;
            this.USBDeviceNameLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // USBInEndpointLabel
            // 
            this.USBInEndpointLabel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.USBInEndpointLabel.AutoSize = true;
            this.USBInEndpointLabel.Location = new System.Drawing.Point(87, 22);
            this.USBInEndpointLabel.Name = "USBInEndpointLabel";
            this.USBInEndpointLabel.Size = new System.Drawing.Size(186, 22);
            this.USBInEndpointLabel.TabIndex = 4;
            this.USBInEndpointLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // USBOutEndpointLabel
            // 
            this.USBOutEndpointLabel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.USBOutEndpointLabel.AutoSize = true;
            this.USBOutEndpointLabel.Location = new System.Drawing.Point(87, 44);
            this.USBOutEndpointLabel.Name = "USBOutEndpointLabel";
            this.USBOutEndpointLabel.Size = new System.Drawing.Size(186, 22);
            this.USBOutEndpointLabel.TabIndex = 5;
            this.USBOutEndpointLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // SourceListBox
            // 
            this.SourceListBox.FormattingEnabled = true;
            this.SourceListBox.Items.AddRange(new object[] {
            "USB 3.0",
            "Memory",
            "File"});
            this.SourceListBox.Location = new System.Drawing.Point(6, 43);
            this.SourceListBox.Name = "SourceListBox";
            this.SourceListBox.Size = new System.Drawing.Size(96, 43);
            this.SourceListBox.TabIndex = 5;
            this.SourceListBox.SelectedIndexChanged += new System.EventHandler(this.SoureListBox_SelectedIndexChanged);
            // 
            // SinkListBox
            // 
            this.SinkListBox.FormattingEnabled = true;
            this.SinkListBox.Items.AddRange(new object[] {
            "USB 3.0",
            "Memory",
            "File"});
            this.SinkListBox.Location = new System.Drawing.Point(357, 44);
            this.SinkListBox.Name = "SinkListBox";
            this.SinkListBox.Size = new System.Drawing.Size(96, 43);
            this.SinkListBox.TabIndex = 6;
            this.SinkListBox.SelectedIndexChanged += new System.EventHandler(this.SinkListBox_SelectedIndexChanged);
            // 
            // TotalBytesTextBox
            // 
            this.TotalBytesTextBox.Location = new System.Drawing.Point(729, 400);
            this.TotalBytesTextBox.Name = "TotalBytesTextBox";
            this.TotalBytesTextBox.ReadOnly = true;
            this.TotalBytesTextBox.Size = new System.Drawing.Size(79, 20);
            this.TotalBytesTextBox.TabIndex = 11;
            this.TotalBytesTextBox.Text = "0";
            this.TotalBytesTextBox.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(612, 403);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(103, 13);
            this.label7.TabIndex = 12;
            this.label7.Text = "Total MB transferred";
            // 
            // SourcePictureBox
            // 
            this.SourcePictureBox.Location = new System.Drawing.Point(108, 16);
            this.SourcePictureBox.Name = "SourcePictureBox";
            this.SourcePictureBox.Size = new System.Drawing.Size(96, 96);
            this.SourcePictureBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.SourcePictureBox.TabIndex = 11;
            this.SourcePictureBox.TabStop = false;
            // 
            // SinkPictureBox
            // 
            this.SinkPictureBox.InitialImage = null;
            this.SinkPictureBox.Location = new System.Drawing.Point(255, 16);
            this.SinkPictureBox.Name = "SinkPictureBox";
            this.SinkPictureBox.Size = new System.Drawing.Size(96, 96);
            this.SinkPictureBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.SinkPictureBox.TabIndex = 10;
            this.SinkPictureBox.TabStop = false;
            // 
            // pictureBox6
            // 
            this.pictureBox6.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox6.Image")));
            this.pictureBox6.Location = new System.Drawing.Point(210, 43);
            this.pictureBox6.Name = "pictureBox6";
            this.pictureBox6.Size = new System.Drawing.Size(39, 42);
            this.pictureBox6.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.pictureBox6.TabIndex = 9;
            this.pictureBox6.TabStop = false;
            // 
            // LinkDefinitionGroupBox
            // 
            this.LinkDefinitionGroupBox.Controls.Add(this.SourcePictureBox);
            this.LinkDefinitionGroupBox.Controls.Add(this.SourceListBox);
            this.LinkDefinitionGroupBox.Controls.Add(this.SinkListBox);
            this.LinkDefinitionGroupBox.Controls.Add(this.SinkPictureBox);
            this.LinkDefinitionGroupBox.Controls.Add(this.pictureBox6);
            this.LinkDefinitionGroupBox.Location = new System.Drawing.Point(12, 103);
            this.LinkDefinitionGroupBox.Name = "LinkDefinitionGroupBox";
            this.LinkDefinitionGroupBox.Size = new System.Drawing.Size(464, 126);
            this.LinkDefinitionGroupBox.TabIndex = 0;
            this.LinkDefinitionGroupBox.TabStop = false;
            this.LinkDefinitionGroupBox.Text = "Link definition";
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.FileName = "openFileDialog1";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(131, 416);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(62, 13);
            this.label8.TabIndex = 13;
            this.label8.Text = "Throughput";
            // 
            // imageList1
            // 
            this.imageList1.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imageList1.ImageStream")));
            this.imageList1.TransparentColor = System.Drawing.Color.Transparent;
            this.imageList1.Images.SetKeyName(0, "USB-FX3_Final.jpg");
            this.imageList1.Images.SetKeyName(1, "Devices-media-flash-icon.png");
            this.imageList1.Images.SetKeyName(2, "Devices-drive-harddisk-icon.png");
            // 
            // CharacterizeButton
            // 
            this.CharacterizeButton.Location = new System.Drawing.Point(805, 189);
            this.CharacterizeButton.Name = "CharacterizeButton";
            this.CharacterizeButton.Size = new System.Drawing.Size(75, 40);
            this.CharacterizeButton.TabIndex = 14;
            this.CharacterizeButton.Text = "Characterize System";
            this.CharacterizeButton.UseVisualStyleBackColor = true;
            this.CharacterizeButton.Click += new System.EventHandler(this.button3_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(892, 453);
            this.Controls.Add(this.CharacterizeButton);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.LinkDefinitionGroupBox);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.TotalBytesTextBox);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.ParametersGroupBox);
            this.Controls.Add(this.ProductionTextBox);
            this.Controls.Add(this.StopButton);
            this.Controls.Add(this.GoButton);
            this.Controls.Add(this.ThroughputChart);
            this.Controls.Add(this.ThroughputGauge);
            this.Name = "Form1";
            this.Text = "Form1";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            ((System.ComponentModel.ISupportInitialize)(this.ThroughputChart)).EndInit();
            this.ParametersGroupBox.ResumeLayout(false);
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.tableLayoutPanel2.ResumeLayout(false);
            this.tableLayoutPanel2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.SourcePictureBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.SinkPictureBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox6)).EndInit();
            this.LinkDefinitionGroupBox.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.AGauge ThroughputGauge;
        private System.Windows.Forms.DataVisualization.Charting.Chart ThroughputChart;
        private System.Windows.Forms.Button GoButton;
        private System.Windows.Forms.Button StopButton;
        private System.Windows.Forms.TextBox ProductionTextBox;
        private System.Windows.Forms.GroupBox ParametersGroupBox;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox PPXComboBox;
        private System.Windows.Forms.ComboBox InterleavedComboBox;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.ComboBox FileBlockSizeComboBox;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label USBDeviceNameLabel;
        private System.Windows.Forms.Label USBInEndpointLabel;
        private System.Windows.Forms.Label USBOutEndpointLabel;
        private System.Windows.Forms.ListBox SinkListBox;
        private System.Windows.Forms.ListBox SourceListBox;
        private System.Windows.Forms.TextBox TotalBytesTextBox;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.PictureBox SourcePictureBox;
        private System.Windows.Forms.PictureBox SinkPictureBox;
        private System.Windows.Forms.PictureBox pictureBox6;
        private System.Windows.Forms.GroupBox LinkDefinitionGroupBox;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.Button SourceFileButton;
        private System.Windows.Forms.Button DestinationFileButton;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.SaveFileDialog saveFileDialog1;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.ImageList imageList1;
        private System.Windows.Forms.Button CharacterizeButton;
    }
}

