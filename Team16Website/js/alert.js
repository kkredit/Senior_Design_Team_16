
function alertSignal(){
	var phone1 = "";
	phone1 = $('input[name="phone-1"]').val()+$('input[name="phone-2"]').val()+$('input[name="phone-3"]').val();
	var phone2 = $('input[name="phone-4"]').val()+$('input[name="phone-5"]').val()+$('input[name="phone-6"]').val();
	var email1 = $('input[name="Email1"]').val();
	var email2 = $('input[name="Email2"]').val();
	console.log(phone2);
	console.log(phone1);

	if(isNaN(phone1) == false && isNaN(phone2) == false){
		if(phone1 == phone2){
			if(!$('input[name="Email1"]').hasClass("invalid") || !$('input[name="Email2"]').hasClass("invalid")){
				if(email1 == email2){

					var jsonString = "email: "+email1+"\nphone_number: "+phone1+"";
					console.log(jsonString);

					$.ajax({
						url: "../alert.php",
						cache: false,
						dataType: "text",
						type: 'POST',
						data: {jsonString: jsonString},
						success: function(data) {
							alert(data);
						}
					});





				}else{
					alert("Email fields do not match.\n Please enter the same email in both forms.");
				}
			}else{
				alert("Not Valid Email\n Please enter a valid email address");
			}
		}else{
			alert("The phone numbers entered do not match.\n Please enter the same phone number in both forms.");
		}
	}else{
		alert("The number entered is not a valid phone number.\n Please enter a valid phone number.");
	}

	


}

