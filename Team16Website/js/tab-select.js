
var tab = null;
$(document).ready(function() {
    $(document).on("click" , ".tab-titles", function() {
        tab = $('.tab-titles').children('a').attr("href");
        $(".tab-content").addClass("hide");
		$(tab).removeClass("hide");
    });
});