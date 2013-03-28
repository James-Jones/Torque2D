$(document).keydown(function(evt){
	if (evt.which==13 && (evt.altKey)){
		evt.preventDefault();
		showTorqueConsole()	
	}
});
