var number = 4;
var tabs = null;
var num = 4;
var tab1num = 4;
var tab2num = 4;
var tab3num = 4;
var i= 0;
$(document).ready(function(){
	tabs=$('#tabs').scrollTabs({
		click_callback: function(e){
			$('div[id^="tab"]').addClass("hide").removeClass("active");
			$('#tabs-container').removeClass("hide");

			var id = $(this).children().attr('href');
			$(id + '.tab-content').removeClass("hide");
			$(id + '.tab-content').addClass("active");
			$('#button4').removeClass("hide");
			$('#button5').removeClass("hide");

			if(id=="#tab1"){
				num = tab1num;
			} else if(id == "#tab2"){
				num = tab2num;
			} else if(id == "#tab3"){
				num = tab3num;
			}


		}
	});
});


$(document).ready(function(){
	$.getJSON('../../garden_net/gn_util/current_schedule_in_db.txt', function(jsondata){
		$.each(jsondata, function(i, item){
			var thingName = "";
			thingName = i;
			$.each(item, function(j, event){
				if(j=="openevents"){
					if(thingName=="Zone1"){
						tab1num=event;
						num = tab1num;
					} else if(thingName=="Zone2"){
						tab2num=event;
					} else if(thingName=="Zone3"){
						tab3num=event;
					}
				}
			});
		});
	
	for(i=0; i<tab1num; i++){
		$('#tab1 #row'+i+'').removeClass("hide");
	}

	for(i=0; i<tab2num; i++){
		$('#tab2 #row'+i+'').removeClass("hide");
	}

	for(i=0; i<tab3num; i++){
		$('#tab3 #row'+i+'').removeClass("hide");
	}



	});

	

});





function forwardFunction()
{
	if(number<16){
		++number; 
		tabs.addTab("<li id='li_"+number+"'><a id ='"+number+"' href='#tab"+number+"'>Zone "+number+"</li>");

		if(number>0){
			$("#button2").show()
		}
	}
	else{
		$("#button").hide();   
	}

}

function deleteTabs()
{ 

	
	if(number>1){
		tabs.removeTabs("#li_"+number);
		--number;

		if(number<17){
			$("#button").show(); 
		}
	}
	else{

		tabs.removeTabs("#li_"+number);
		--number;

		$('.scroll_tab_left_finisher').remove();
		$('.scroll_tab_right_finisher').remove();
		$("#button2").hide();
	}


}
function submit(){
	for(i=tab1num-1; i<22; i++ ){
		$('select[name="Zone1[event'+i+'[day]]"').val("");
		$('input[name="Zone1[event'+i+'[start_time]]"').val("");
		$('input[name="Zone1[event'+i+'[stop_time]]"').val("");
	}
	for(i=tab2num-1; i<22; i++ ){
		$('select[name="Zone2[event'+i+'[day]]"').val("");
		$('input[name="Zone2[event'+i+'[start_time]]"').val("");
		$('input[name="Zone2[event'+i+'[stop_time]]"').val("");
	}
	for(i=tab3num-1; i<22; i++ ){
		$('select[name="Zone3[event'+i+'[day]]"').val("");
		$('input[name="Zone3[event'+i+'[start_time]]"').val("");
		$('input[name="Zone3[event'+i+'[stop_time]]"').val("");
	}






	var myForm = $("#myForm").serializeJSON();
	var jsonString = JSON.stringify(myForm);
	//console.log(jsonString);

	$.ajax({
		url: "../savejson.php",
		cache: false,
		dataType: "text",
		type: 'POST',
		data: {jsonString: jsonString},
		success: function(data) {
			alert(data);
		}
		

	});
	

}



function addEvent1(){
	var active = $('div').hasClass("tab-content card-content active");
	
		$('#button5').removeClass('hide');
		if(num<12){
			($('div.active').find('#row'+num+'')).removeClass('hide');
			++num;
		}else{
			$('#button4').addClass("hide");
			
		}
		
		if($('#tab1').hasClass("active") == true){
			tab1num = num;
			$('input[name="Zone1[openevents]"]').val(tab1num);
		} else if($('#tab2').hasClass("active") == true){
			tab2num = num;
			$('input[name="Zone2[openevents]"]').val(tab2num);
		} else if($('#tab3').hasClass("active") == true){
			tab3num = num;
			$('input[name="Zone3[openevents]"]').val(tab3num);
		}

	
}

function deleteEvent1(){
	var active = $('div').hasClass("tab-content card-content active");

	
		$('#button4').removeClass('hide');
		--num;
		if(num>1){
			
			($('div.active').find('#row'+(num)+'')).addClass('hide');
			
			
		}else{
			($('div.active').find('#row'+(num)+'')).addClass('hide');
			$('#button5').addClass('hide');
			
		}
		if($('#tab1').hasClass("active") == true){
			tab1num = num;
			$('input[name="Zone1[openevents]"]').val(tab1num);
		} else if($('#tab2').hasClass("active") == true){
			tab2num = num;
			$('input[name="Zone2[openevents]"]').val(tab2num);
		} else if($('#tab3').hasClass("active") == true){
			tab3num = num;
			$('input[name="Zone3[openevents]"]').val(tab3num);
		}
		

	
}

function switchCheck(){

	var checkbox = $('#mySwitch').prop('checked');
	var string = JSON.stringify(checkbox);
	console.log(string);	
	//alert('checkbox value: '+ checkbox);
	$.ajax({
		url: "../savejson.php",
		cache: false,
		dataType: "text",
		type: 'POST',
		data: {string: string},
		success: function(data) {
			alert(data);
		}
		

	});

}







