<template>
    <div>
        <button v-on:click="download_csv_file(spectral_data)">Generate Report</button>
    </div>
</template>

<script>
export default {
    data () {
        return {
            temp0:0,
            temp1:0,
            temp2:0,
            csvFileData:[]
        }
    },
  props: {
    spectral_data: {
      type: Object,
      required: true
    },
  },
  created:
    function () {
      this.$parent.subscribe('/thermistor_data', (msg) => {
        this.temp0 = msg.temp0
        this.temp1 = msg.temp1
        this.temp2 = msg.temp2
      })
    },
    methods:{    
    download_csv_file: function(spectral_data) {  
        const time = new Date(Date.now())
        const timeString = time.toTimeString().substring(0,17) +" "+time.toDateString()
        this.csvFileData = [  
        [timeString, spectral_data.d0_1,spectral_data.d1_1,spectral_data.d2_1,this.temp0],  
        [timeString, spectral_data.d0_2,spectral_data.d1_2,spectral_data.d2_2,this.temp1],  
        [timeString, spectral_data.d0_3,spectral_data.d1_3,spectral_data.d2_3,this.temp2],  
        [timeString, spectral_data.d0_4,spectral_data.d1_4,spectral_data.d2_4,],  
        [timeString, spectral_data.d0_5,spectral_data.d1_5,spectral_data.d2_5,],  
        [timeString, spectral_data.d0_6,spectral_data.d1_6,spectral_data.d2_6,]
        ];  
        //define the heading for each row of the data  
        var csv = 'Timestamp,Spectral 0,Spectral 1, Spectral 2, Temps\n';  
        
        //merge the data with CSV  
        this.csvFileData.forEach(function(row) {  
                csv += row.join(',');  
                csv += "\n";  
        });  
          
        
        var hiddenElement = document.createElement('a');  
        hiddenElement.href = 'data:text/csv;charset=utf-8,' + encodeURI(csv);  
        hiddenElement.target = '_blank';  
        
        //provide the name for the CSV file to be downloaded  
        let report_name = prompt("Enter filename:", timeString);  
        hiddenElement.download = report_name+'.csv';  
        hiddenElement.click();  
    }  
    }
}
</script>

<style scoped>
  button {
    width: 17vh;
    height: 7vh;
  }
</style>