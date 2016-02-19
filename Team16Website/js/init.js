(function($){
  $(function(){

    $('.button-collapse').sideNav({
	  menuWidth: 300, // Default is 240
      edge: 'left', // Choose the horizontal origin
      closeOnClick: true // Closes side-nav on <a> clicks, useful for Angular/Meteor
	});


	$(document).ready(function() {
    $('.collapsible').collapsible({
      accordion : false // A setting that changes the collapsible behavior to expandable instead of the default accordion style
    });
	$('ul.tabs').tabs();
	$('.parallax').parallax();
	$(".dropdown-button").dropdown();
	$('select').material_select();

	
});

  }); // end of document ready
})(jQuery); // end of jQuery name space