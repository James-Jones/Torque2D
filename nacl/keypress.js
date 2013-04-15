
$(document).keydown(function(evt){
	if (evt.which==13 && (evt.altKey)){//Alt+return
		evt.preventDefault();
		showFile('/main/console.log');
	}
});
