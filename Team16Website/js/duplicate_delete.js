 var number = 4;
 var tabs = null;
 $(document).ready(function(){
	 tabs=$('#tabs').scrollTabs({
		 click_callback: function(e){
        	$(".tab-content").addClass("hide");
			$("a[href]").addClass("active");
			$(".active").removeClass("hide");
		 }
	 });
 });
 
 
 function forwardFunction()
   {
       if(number<16){
       ++number; 

       tabs.addTab("<li id='li_"+number+"'><a id ='"+number+"' href='#tab"+number+"'>Tab "+number+"</li>");

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
 
 
