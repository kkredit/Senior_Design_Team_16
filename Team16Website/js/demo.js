$(document).ready(function(){
	$("select").material_select();
	tabs=$('#tabs').scrollTabs({
		click_callback: function(e){
			$('div[id^="demo"]').addClass("hide").removeClass("active");
			$('#tabs-container').removeClass("hide");
			var id = $(this).children().attr('href');
			$(id + '.tab-content').removeClass("hide");
			$(id + '.tab-content').addClass("active");

		}
	});
});


function submit(){
	var myForm = $("#demoForm").serializeJSON();
	var jsonString = JSON.stringify(myForm);
	console.log(jsonString);

	for(i=0; i<3; i++){
		for(j=0; j<3; j++){
			var str1 = $('input[name="Zone'+i+'[event'+j+'[start_time]]"]').val();
			var str2 = $('input[name="Zone'+i+'[event'+j+'[stop_time]]"]').val();

			if(str1 > str2){
				alert("ERROR: Cannot have stop time earlier than start time\n Error located in Zone "+i+" event "+(j+1)+"");
				return;
			}
			
		}

	}


	$.ajax({
		url: "../savejson.php",
		cache: false,
		dataType: "text",
		type: 'POST',
		data: {jsonString: jsonString},
		success: function(data) {
			alert("Garden Info Submitted!");
		}
		

	});
}