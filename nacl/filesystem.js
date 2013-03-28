	var resourceURL;
	var resourceDIRLOC;
	var fileSystem;
	var resourceDIR;

	function errorHandler(e) {
	  var msg = '';
      console.dir(e);
	  switch (e.code) {
	    case FileError.QUOTA_EXCEEDED_ERR:
	      msg = 'QUOTA_EXCEEDED_ERR';
	      break;
	    case FileError.NOT_FOUND_ERR:
	      msg = 'NOT_FOUND_ERR';
	      break;
	    case FileError.SECURITY_ERR:
	      msg = 'SECURITY_ERR';
	      break;
	    case FileError.INVALID_MODIFICATION_ERR:
	      msg = 'INVALID_MODIFICATION_ERR';
	      break;
	    case FileError.INVALID_STATE_ERR:
	      msg = 'INVALID_STATE_ERR';
	      break;
	    default:
	      msg = 'Unknown Error';
	      break;
	  };

	  console.log('Error: ' + msg);
	}

	function fetchResource() {
		var xhr = new XMLHttpRequest();
		xhr.responseType="arraybuffer";
		xhr.open("GET", resourceURL,true);
		xhr.onload = function(e) {
			if(this.status == 200) {
				//store the last mod for this file
				localStorage["resourceLastModified"] = this.getResponseHeader("Last-Modified");
				
				var content = new Int8Array(this.response);
				
				var blob = new Blob([content], {type: 'application/zip'});

				zip.workerScriptsPath = "zip/";

				zip.createReader(new zip.BlobReader(blob), function(reader) {

					reader.getEntries(function(entries) {

							entries.forEach(function(entry) {
								resourceDIR.getFile(entry.filename, {create:true}, function(file) {
									entry.getData(new zip.FileWriter(file), function(e) {

									  }, function(current, total) {
										// onprogress callback
									  });

								});
							});

					});
				}, function(err) {
					console.log("zip reader error!");
					console.dir(err);
				})
			}
		}
		xhr.send();
	}

	function initLocalStorage(sizeInBytes, appName, assetZipName) {

        if(!window.webkitStorageInfo) return;
		
		resourceURL = assetZipName;
		resourceDIRLOC = appName;

		window.webkitStorageInfo.requestQuota(window.PERSISTENT, sizeInBytes, function(grantedBytes) {
			console.log("I was granted "+grantedBytes+" bytes.");
			window.webkitRequestFileSystem(window.PERSISTENT, grantedBytes, onInitFs, errorHandler);
		}, errorHandler);

		function onInitFs(fs) {
			fileSystem = fs;

			fileSystem.root.getDirectory(fs.root.fullPath + '/' + resourceDIRLOC, {create:true}, function(dir) {
				resourceDIR = dir;

				//So we have our resource folder. To determine if our copy is in sync, we check localStorage and compare it to the remote file
				if(localStorage["resourceLastModified"]) {
					console.log("I had a localStorage value for resourceLastModified, it was "+localStorage["resourceLastModified"]);
					var xhr = new XMLHttpRequest();
					xhr.open("HEAD", resourceURL );
					xhr.onload = function(e) {
						if(this.status == 200) {
							if(this.getResponseHeader("Last-Modified") != localStorage["resourceLastModified"]) {
								fetchResource();
							} else {
								console.log("Not fetching the zip, already have latest version.");
							}
						}
					}
					xhr.send();
				} else {
					fetchResource();
				}

			},errorHandler);
		}	
	}
	
function showTorqueConsole() {

  fileSystem.root.getFile('/myapp/console.log', {}, function(fileEntry) {

    // Get a File object representing the file,
    // then use FileReader to read its contents.
    fileEntry.file(function(file) {
       var reader = new FileReader();

       reader.onloadend = function(e) {
			createTorqueConsoleWindow(this.result);		
       };

       reader.readAsText(file);
    }, errorHandler);

  }, errorHandler);

}
