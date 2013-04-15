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
	
function createDirAndFile(zipEntry, rootDirEntry, folders) {
  // Throw out './' or '/' and move on to prevent something like '/foo/.//bar'.
  if (folders[0] == '.' || folders[0] == '') {
    folders = folders.slice(1);
  }
  rootDirEntry.getDirectory(folders[0], {create: true}, function(dirEntry) {
	//console.log("Created " + folders[0]);
    // Recursively add the new subfolder (if we still have another to create).
    if (folders.length) {
      createDirAndFile(zipEntry, rootDirEntry, folders.slice(1));
    }
	else
	{
		//All dirs created
		console.log("Request ", zipEntry.filename);
		
		try {
		rootDirEntry.getFile(zipEntry.filename, {create:true}, function(file) {

				console.log("Got " + file.fullPath);
			
			try {
				zipEntry.getData(new zip.FileWriter(file), function(e) {

			  });
		  } catch(e) {
			console.log(e);
			}
		  },
		  errorHandler);
	  } catch(e) {
		console.log(e);
	  }
	}
  }, errorHandler);
}

function writeZipEntryToDisk(entry) {
	var n = entry.filename.lastIndexOf("/");
	var dirName = entry.filename.slice(0, n);
	//console.log("Filename = ", entry.filename.slice(0, n));

	createDirAndFile(entry, resourceDIR, dirName.split("/"));
}

function fetchResource(callback) {
	var xhr = new XMLHttpRequest();
	xhr.responseType="arraybuffer";
	xhr.open("GET", resourceURL,true);
	xhr.onload = function(e) {
		if(this.status == 200) {
			//store the last mod for this file
			localStorage.setItem("resourceLastModified", this.getResponseHeader("Last-Modified"));
			
			var content = new Int8Array(this.response);
			
			var blob = new Blob([content], {type: 'application/zip'});

			
			fileSystem.root.getFile("/zip/Torque2DNaClAssets.zip", {create: true}, function(fileEntry) {

				// Create a FileWriter object for our FileEntry (log.txt).
				fileEntry.createWriter(function(fileWriter) {

				  fileWriter.onwriteend = function(e) {
					//console.log('Write completed.');
					  callback();
				  };

				  fileWriter.onerror = function(e) {
					console.log('Write failed: ' + e.toString());
				  };

				  //fileWrite.truncate(0);
				  fileWriter.write(blob);

				}, errorHandler);

			}, errorHandler);
	
			/*zip.workerScriptsPath = "zip/";

			zip.createReader(new zip.BlobReader(blob), function(reader) {

				reader.getEntries(function(entries) {

						entries.forEach(writeZipEntryToDisk,
							errorHandler);
							
						});

				//});
			}, function(err) {
				console.log("zip reader error!");
				console.dir(err);
			})*/
		}
	}
	xhr.send();
}

function removeExistingFiles(fs) {
  fs.root.getDirectory('/main', {}, function(dirEntry) {

    dirEntry.removeRecursively(function() {
      console.log('Main directory removed.');
    }, errorHandler);

  }, errorHandler);
  
   fs.root.getDirectory('/zip', {}, function(dirEntry) {

    dirEntry.removeRecursively(function() {
      console.log('Zip directory removed.');
    }, errorHandler);

  }, errorHandler);
  
  fs.root.getDirectory('/dirdump', {}, function(dirEntry) {

    dirEntry.removeRecursively(function() {
      console.log('dirdump directory removed.');
    }, errorHandler);

  }, errorHandler);
}

function initLocalStorage(sizeInBytes, appName, assetZipName, callback) {

	if(!window.webkitStorageInfo) return;
	
	resourceURL = assetZipName;
	resourceDIRLOC = appName;

	window.webkitStorageInfo.requestQuota(window.PERSISTENT, sizeInBytes, function(grantedBytes) {
		console.log("I was granted "+grantedBytes+" bytes.");
		window.webkitRequestFileSystem(window.PERSISTENT, grantedBytes, onInitFs, errorHandler);
	}, errorHandler);

	function onInitFs(fs) {
		fileSystem = fs;
		
		//Remove existing
		//removeExistingFiles(fs);
		
		//Create /main.
		//Create /zip
		//Put the latest asset zip file into /zip dir
		//Call the callback
		fileSystem.root.getDirectory(fs.root.fullPath + '/main', {create:true}, function(mainDir) {

			fileSystem.root.getDirectory(fs.root.fullPath + '/zip', {create:true}, function(zipDir) {
				resourceDIR = zipDir;
				
				localStorage.removeItem("resourceLastModified");

				//So we have our resource folder. To determine if our copy is in sync, we check localStorage and compare it to the remote file
				if(localStorage.getItem("resourceLastModified")) {
					console.log("I had a localStorage value for resourceLastModified, it was "+localStorage.getItem("resourceLastModified"));
					var xhr = new XMLHttpRequest();
					xhr.open("HEAD", resourceURL );
					xhr.onload = function(e) {
						if(this.status == 200) {
							if(this.getResponseHeader("Last-Modified") != localStorage.getItem("resourceLastModified")) {
								fetchResource(callback);
							} else {
								console.log("Not fetching the zip, already have latest version.");
								callback();
							}
						}
					}
					xhr.send();
				} else {
					fetchResource(callback);
				}

			},errorHandler);
			
			//fileSystem.root.getFile("/zip/Torque2DNaClAssets.zip", {create: true}, function(fileEntry) {
			//	fileEntry.getMetadata(function(mdata) {
			//		console.log("Size = " + mdata.size);
			//		console.log("Time = " + mdata.modificationTime);
			//	});
			//});
		},errorHandler);
	}	
}

function showFile(fileToShow) {

  fileSystem.root.getFile(fileToShow, {}, function(fileEntry) {

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

function writeFile(filename, contents) {
	fileSystem.root.getFile(filename, {create: true}, function(fileEntry) {

		// Create a FileWriter object for our FileEntry (log.txt).
		fileEntry.createWriter(function(fileWriter) {

		  fileWriter.onwriteend = function(e) {
			//console.log('Write completed.');
		  };

		  fileWriter.onerror = function(e) {
			console.log('Write failed: ' + e.toString());
		  };

		  // Create a new Blob and write it to log.txt.
		  var blob = new Blob([contents], {type: 'text/plain'});

		  //fileWrite.truncate(0);
		  fileWriter.write(blob);

		}, errorHandler);

	}, errorHandler);
}

function dumpDirectories(rootPath) {
	fileSystem.root.getDirectory(rootPath, {}, function(dirEntry){
		var dirReader = dirEntry.createReader();
		dirReader.readEntries(function(entries) {
			var dirString = ""
			for(var i = 0; i < entries.length; i++) {
				var entry = entries[i];
				if (entry.isDirectory){
					console.log('Directory: ' + entry.fullPath);
					dirString += 'Directory: ' + entry.fullPath + '\n';
				}
				else if (entry.isFile){
					console.log('File: ' + entry.fullPath);
					dirString += 'File: ' + entry.fullPath + '\n';
				}
			}
			
			//Now write the dir list to a file so Torque2D c++ engine code can access it.
			//writeFile('/main/dumpDir.txt', dirString)
		}, errorHandler);
	}, errorHandler);
}
