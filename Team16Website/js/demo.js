 // -- Beginning of file --
/* 
 * [demo.js]
 * 
 * demo.js handles the form entry fields for Demo 2 for Senior Design Night
 * Fields are treated as if they were the same as the normal schedule.
 *
 * (C) 2016, John Connell
 * Last Modified: [29-04-2016]
 */








$(document).ready(function(){
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

/* 
 * submit()
 *
 * Takes input form of demo.html and submits them with error checking.
 * 
 * 
 * @param text str1: values of each form's start time
 * @param text str2: values of each form's end time
 * ^^ repeat for each parameter^^
 * 
 * @return type varname: returns alert of "Demo Submitted!" upon successful submission.
 */ 

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
		url: "../demo2.php",
		cache: false,
		dataType: "text",
		type: 'POST',
		data: {jsonString: jsonString},
		success: function(data) {
			alert("Demo Submitted!");
		}
		

	});
}