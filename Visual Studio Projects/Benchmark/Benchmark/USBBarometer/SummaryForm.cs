using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace USBBarometer
{
    public partial class SummaryForm : Form
    {
        public SummaryForm(List<Tuple<String, double>> displayData)
        {
            InitializeComponent();

            foreach (var item in displayData)
            {
                dataGridView1.Rows.Add(new object[] { item.Item1, item.Item2 });
            }
        }
    }
}
