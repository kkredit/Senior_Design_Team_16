
google.charts.load('current', {'packages':['corechart']});
google.charts.setOnLoadCallback(drawChart);

var flowArray = [];
var nodeValveArray = [];


var intFlow;
var rest;
var data, dates;
var GpercentUp = [];
var MeshUp = [];

var node = [];













function drawChart() {
	var data2 = new google.visualization.DataTable();
	var data1 = new google.visualization.DataTable();
	var dateArray = [];
	var numNodes = 0;
	var numValves = 0;
	var lines = [];
	var noNodeDates = [];

	data2.addColumn('date', 'Day');
	data1.addColumn('date', 'Day');
	


	var url = "/var/www/Team16Website/garden_net/gn_util/historical_data.txt";
	var info = "";
	$.ajax({
		type: 'POST',
		url: "/GardeNetControl/phppage.php",
		data: {url:url},
		dataType: "text",
		async: false,
		success: function(data){
			info = data;
			dates = info.split("\n\n\n");
			for(var i = 0; i<dates.length; i++){
				lines.push(dates[i].split("\n"));
			}
			
			var nodeLine = [];
			var valveLine = [];
			dates.pop();
			lines.pop();

			var noNodes = 0;
			
			


			for(var i = 0; i<dates.length; i++){
				if(lines[i].length < 8){
						++noNodes;
						noNodeDates.push(i);
					}
				for(var j = 0; j<lines[i].length; j++){ 
					
					
					if(lines[i][j].indexOf("node: ") > -1){
						nodeLine.push(lines[i][j]);
					}else if(((lines[i][j]).indexOf("valve_num:")) > -1){
						valveLine.push(lines[i][j]);

					}
				}				
			}

			numValves = valveLine.length/(dates.length - noNodes);
			numNodes = nodeLine.length/(dates.length - noNodes);
			console.log(numValves);
			console.log(numNodes);
			for(var i=1; i<numNodes+1; i++){
				data2.addColumn('number', "Flow Node "+[i]+"");
				data2.addColumn('number', "Cumulative Flow Node "+[i]+"");
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
							if(numNodes==1){
								count[0] = 0;
							}else{
								count[0] = 1;
							}
							
							
						}else if(count[0] == 1){
							nodeValveArray[1][0].push(parseFloat(valveLine[i].split("totalWateringTime: ")[1]));
							if(numNodes==2){
								count[0] = 0;
							}else{
								count[0] = 2;
							}
							
						}else if(count[0] == 2){
							nodeValveArray[2][0].push(parseFloat(valveLine[i].split("totalWateringTime: ")[1]));
							count[0] = 0;
						}
						
						
					}else if(valveLine[i].indexOf("valve_num: 2")> -1){
						if(count[1] == 0){
							nodeValveArray[0][1].push(parseFloat(valveLine[i].split("totalWateringTime: ")[1]));
							if(numNodes==1){
								count[1] = 0;
							}else{
								count[1] = 1;
							}
						}else if(count[1] == 1){
							nodeValveArray[1][1].push(parseFloat(valveLine[i].split("totalWateringTime: ")[1]));
							if(numNodes==2){
								count[1] = 0;
							}else{
								count[1] = 2;
							}
						}else if(count[1] == 2){
							nodeValveArray[2][1].push(parseFloat(valveLine[i].split("totalWateringTime: ")[1]));
							count[1]=0;
						}
						
					}else if(valveLine[i].indexOf("valve_num: 3")> -1){
						if(count[2] == 0){
							nodeValveArray[0][2].push(parseFloat(valveLine[i].split("totalWateringTime: ")[1]));
							if(numNodes==1){
								count[2] = 0;
							}else{
								count[2] = 1;
							}
						}else if(count[2] == 1){
							nodeValveArray[1][2].push(parseFloat(valveLine[i].split("totalWateringTime: ")[1]));
							if(numNodes==2){
								count[2] = 0;
							}else{
								count[2] = 2;
							}
						}else if(count[2] == 2){
							nodeValveArray[2][2].push(parseFloat(valveLine[i].split("totalWateringTime: ")[1]));
							count[2]=0;
						}
						
					}else if(valveLine[i].indexOf("valve_num: 4")> -1){
						if(count[3] == 0){
							nodeValveArray[0][3].push(parseFloat(valveLine[i].split("totalWateringTime: ")[1]));
							if(numNodes==1){
								count[3] = 0;
							}else{
								count[3] = 1;
							}
						}else if(count[3] == 1){
							nodeValveArray[1][3].push(parseFloat(valveLine[i].split("totalWateringTime: ")[1]));
							if(numNodes==2){
								count[3] = 0;
							}else{
								count[3] = 2;
							}
						}else if(count[3] == 2){
							nodeValveArray[2][3].push(parseFloat(valveLine[i].split("totalWateringTime: ")[1]));
							count[3]=0;
						}
						
					}	
				
			}
			console.log(nodeValveArray);

			for(var i=0; i<dates.length; i++){


				rest = dates[i].split("\t", 4);


				dateArray.push((rest[0].split(" ")[1]));
				GpercentUp.push(parseFloat((rest[3].split(" "))[1]));
				MeshUp.push(parseFloat((rest[2].split(" "))[1]));


			}

			console.log(dateArray);

        
        

    }
	});
	var duplicates = [];
	var cumulativeFlowArray = [];
	for(var i = 0; i<numNodes; i++){
		cumulativeFlowArray[i]= new Array();
	}
	

	console.log(dateArray.length);
	console.log(noNodeDates);
	

	dateArray = $.grep(dateArray, function(n, i) {
	    return $.inArray(i, noNodeDates) ==-1;
	});
	console.log(dateArray);
	for(var i = 0; i<dateArray.length; i++){
		if(dateArray[i+1] == dateArray[i]){
			duplicates.push(i);
		}

		
	}
	console.log(duplicates);
	console.log(duplicates.length);
	dateArray = $.grep(dateArray, function(n, i) {
	    return $.inArray(i, duplicates) ==-1;
	});
	for(var i = 0; i<numNodes; i++){
		for(var j = 0; j<numValves; j++){
			nodeValveArray[i][j] = $.grep(nodeValveArray[i][j], function(n, i) {
						    return $.inArray(i, duplicates) ==-1;
						});
		}
	}

	console.log(dateArray);
	
	console.log(nodeValveArray);
	for(var i=0; i<numNodes; i++){
		for(var j = 0; j<dateArray.length; j++){

			if(j==0){
				cumulativeFlowArray[i].push(flowArray[i][j]);
			}else{
				cumulativeFlowArray[i].push((flowArray[i][j] + cumulativeFlowArray[i][j-1]));
			}


			


		}

	}



	console.log(cumulativeFlowArray);
	console.log(flowArray);

	for(var i=0; i<dateArray.length+1; i++){
		
		if(numNodes== 1){
			data2.addRows([[new Date(dateArray[i]), flowArray[0][i], cumulativeFlowArray[0][i]]]);
			data1.addRows([[new Date(dateArray[i]), nodeValveArray[0][0][i], nodeValveArray[0][1][i], nodeValveArray[0][2][i]]]);
		}else if(numNodes == 2){
			data2.addRows([[new Date(dateArray[i]), flowArray[0][i], cumulativeFlowArray[0][i], flowArray[1][i], cumulativeFlowArray[1][i]]]);
			data1.addRows([[new Date(dateArray[i]), nodeValveArray[0][0][i], nodeValveArray[0][1][i], nodeValveArray[0][2][i], nodeValveArray[1][0][i], nodeValveArray[1][1][i], nodeValveArray[1][2][i] ]]);
		}else if(numNodes == 3){
			data2.addRows([[new Date(dateArray[i]), flowArray[0][i], cumulativeFlowArray[0][i], flowArray[1][i], cumulativeFlowArray[1][i], flowArray[2][i], cumulativeFlowArray[2][i]]]);
			data1.addRows([[new Date(dateArray[i]), nodeValveArray[0][0][i], nodeValveArray[0][1][i], nodeValveArray[0][2][i], nodeValveArray[1][0][i], nodeValveArray[1][1][i], nodeValveArray[1][2][i], nodeValveArray[2][0][i], nodeValveArray[2][1][i], nodeValveArray[2][2][i] ]]);
		}
		

	}
	console.log(numNodes);







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
		},
		pointSize: 10,
		series: {
                0: { pointShape: 'circle' },
                1: { pointShape: 'triangle' },
                2: { pointShape: 'square' },
                3: { pointShape: 'diamond' },
                4: { pointShape: 'star' },
                5: { pointShape: 'polygon' }
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
		},
		pointSize: 10,
		series: {
                0: { pointShape: 'circle' },
                1: { pointShape: 'triangle' },
                2: { pointShape: 'square' },
                3: { pointShape: 'diamond' },
                4: { pointShape: 'star' },
                5: { pointShape: 'polygon' }
        }
	};




	var chart = new google.visualization.LineChart(document.getElementById('chart'));
	var chart1 = new google.visualization.LineChart(document.getElementById('chart1'));

	chart.draw(data2, options);
	chart1.draw(data1, options1);
	console.log(MeshUp[MeshUp.length-1]);
	

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

	$('span#MeshUptime').text((averageMesh).toString());
	$('span#GUptime').text((average3G).toString());

}

function deleteData(){
	data = "";

	$.ajax({
		type: 'POST',
		url: "/GardeNetControl/deletedata.php",
		data: {data:data},
		dataType: "text"
	});

}

