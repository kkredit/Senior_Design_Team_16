
google.charts.load('current', {'packages':['corechart']});
google.charts.setOnLoadCallback(drawChart);

var flowArray = [];
var nodeValveArray = [];


var intFlow;
var rest;
var data, dates;
var GpercentUp = [];
var MeshUp = [];
var dateArray = [];
var node = [];













function drawChart() {
	var data2 = new google.visualization.DataTable();
	var data1 = new google.visualization.DataTable();

	data2.addColumn('date', 'Day');
	data1.addColumn('date', 'Day');
	


	var url = "/var/www/Team16Website/GardeNetControl/historical_data_format.txt";
	var info = "";
	$.ajax({
		type: 'POST',
		url: "../phppage.php",
		data: {url:url},
		dataType: "text",
		async: false,
		success: function(data){
			info = data;
			dates = info.split("\n\n");
			var lines = info.split("\n");
			var nodeLine = [];
			var valveLine = [];




			for(var i=lines.length-1; i>=0; i--){
				if(((lines[i]).indexOf("node:")) > -1){
					nodeLine.push(lines[i]);
				}else if(((lines[i]).indexOf("valve_num:")) > -1){
					valveLine.push(lines[i]);

				}
			}
			nodeLine.reverse();
			valveLine.reverse();

			var numValves = valveLine.length/(dates.length);
			var numNodes = nodeLine.length/(dates.length);
			console.log(numValves);
			console.log(numNodes);
			console.log(valveLine.length);
			for(var i=1; i<numNodes+1; i++){
				data2.addColumn('number', "Flow Node "+[i]+"");
				for(var j=1; j<(numValves/numNodes)+1; j++){
					data1.addColumn('number', "Node"+[i]+", Valve "+[j]+"");
				}
			}
			


			for(var i=0; i<numNodes; i++){
				flowArray[i] = new Array();
				nodeValveArray[i] = new Array();
				for(var j=0; j<numValves; j++){
					nodeValveArray[i][j] = new Array();
				}
			}

			for(var i=0; i<nodeLine.length; i++){
				if(nodeLine[i].indexOf("node: 1")> -1){
					flowArray[0].push(parseFloat(nodeLine[i].split("accumulatedFlow: ")[1]));
					
				}else if(nodeLine[i].indexOf("node: 2")> -1){
					flowArray[1].push(parseFloat(nodeLine[i].split("accumulatedFlow: ")[1]));
					
				}else if(nodeLine[i].indexOf("node: 3")> -1){
					flowArray[2].push(parseFloat(nodeLine[i].split("accumulatedFlow: ")[1]));
				}

			}

			/* Separating Valve Line by Nodes. Note if more nodes were created and more valves,
				editing to the original documents would be needed.




			*/

			var count = [0,0,0,0];
			for(var i=0; i<valveLine.length; i++){
					
					if(valveLine[i].indexOf("valve_num: 1")> -1){
						if(count[0] == 0){
							nodeValveArray[0][0].push(parseFloat(valveLine[i].split("totalWateringTime: ")[1]));
							count[0] = 1;
							
						}else if(count[0] == 1){
							nodeValveArray[1][0].push(parseFloat(valveLine[i].split("totalWateringTime: ")[1]));
							count[0] = 2;
						}else if(count[0] == 2){
							nodeValveArray[2][0].push(parseFloat(valveLine[i].split("totalWateringTime: ")[1]));
							count[0] = 0;
						}
						
						
					}else if(valveLine[i].indexOf("valve_num: 2")> -1){
						if(count[1] == 0){
							nodeValveArray[0][1].push(parseFloat(valveLine[i].split("totalWateringTime: ")[1]));
							count[1]=1;
						}else if(count[1] == 1){
							nodeValveArray[1][1].push(parseFloat(valveLine[i].split("totalWateringTime: ")[1]));
							count[1]=2;
						}else if(count[1] == 2){
							nodeValveArray[2][1].push(parseFloat(valveLine[i].split("totalWateringTime: ")[1]));
							count[1]=0;
						}
						
					}else if(valveLine[i].indexOf("valve_num: 3")> -1){
						if(count[2] == 0){
							nodeValveArray[0][2].push(parseFloat(valveLine[i].split("totalWateringTime: ")[1]));
							count[2]=1;
						}else if(count[2] == 1){
							nodeValveArray[1][2].push(parseFloat(valveLine[i].split("totalWateringTime: ")[1]));
							count[2]=2;
						}else if(count[2] == 2){
							nodeValveArray[2][2].push(parseFloat(valveLine[i].split("totalWateringTime: ")[1]));
							count[2]=0;
						}
						
					}else if(valveLine[i].indexOf("valve_num: 4")> -1){
						if(count[3] == 0){
							nodeValveArray[0][3].push(parseFloat(valveLine[i].split("totalWateringTime: ")[1]));
							count[3]=1;
						}else if(count[3] == 1){
							nodeValveArray[1][3].push(parseFloat(valveLine[i].split("totalWateringTime: ")[1]));
							count[3]=2;
						}else if(count[3] == 2){
							nodeValveArray[2][3].push(parseFloat(valveLine[i].split("totalWateringTime: ")[1]));
							count[3]=0;
						}
						
					}	
				
			}
			console.log(nodeValveArray);





			




			for(var i=0; i<dates.length; i++){

				rest = dates[i].split("\t", 4);


				dateArray.push((rest[0]));
				GpercentUp.push(parseFloat((rest[3].split(" "))[1]));
				MeshUp.push(parseFloat((rest[2].split(" "))[1]));


			}

        
        

    }
	});


	for(var i=0; i<dateArray.length+2; i++){
		data2.addRows([[new Date(dateArray[i]), flowArray[0][i], flowArray[1][i], flowArray[2][i]]]);
		data1.addRows([[new Date(dateArray[i]), nodeValveArray[0][0][i], nodeValveArray[0][1][i], nodeValveArray[0][2][i], nodeValveArray[1][0][i], nodeValveArray[1][1][i], nodeValveArray[1][2][i], nodeValveArray[2][0][i], nodeValveArray[2][1][i], nodeValveArray[2][2][i] ]]);

	}






	var options = {
		title: "Water Usage",
		legend: { position: 'right' },
		hAxis: {
			title: 'Day'
		},
		vAxis: {
			title: 'Gallons'
		},
		titleTextStyle: {
			color: '00000',
			fontName: 'Arial',
			fontSize: 20
		}

	};

	var options1 = {
		title: "Valve Usage (Time)",
		legend: {position: 'right'},
		hAxis: {
			title: 'Day'
		},
		vAxis: {
			title: 'Time (Minutes)'
		},
		titleTextStyle: {
			color: '00000',
			fontName: 'Arial',
			fontSize: 20
		}
	};




	var chart = new google.visualization.LineChart(document.getElementById('chart'));
	var chart1 = new google.visualization.LineChart(document.getElementById('chart1'));

	chart.draw(data2, options);
	chart1.draw(data1, options1);
	console.log(MeshUp[MeshUp.length-1]);
	console.log();

	var sumMesh = 0.0;
	var sum3G = 0.0;
	for( var i = 0; i < MeshUp.length; i++ ){
		sumMesh +=  MeshUp[i];
	    sum3G += GpercentUp[i]; //don't forget to add the base
	}

	var averageMesh = sumMesh/MeshUp.length;
	var average3G = sum3G/GpercentUp.length;
	average3G = Math.round(average3G * 100) / 100;
	averageMesh = Math.round(averageMesh * 100) / 100;

	$('span#MeshUptime').text((averageMesh*100).toString());
	$('span#GUptime').text((average3G*100).toString());

}


