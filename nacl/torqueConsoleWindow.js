var myWindow;

function createTorqueConsoleWindow(consoleString) {
	myWindow=window.open('','','width=640,height=480')
	
	var txtArea = myWindow.document.createElement('textarea');
	txtArea.readonly = true;
	txtArea.cols = 200;
	txtArea.rows = 100;
	txtArea.value = consoleString;
	myWindow.document.body.appendChild(txtArea);
	
	//myWindow.document.write(consoleString)
	
	myWindow.focus()
}
