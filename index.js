
// Node.js program to demonstrate the  
// response.setHeaders() Method 
  
// Importing http module 
var http = require('http'); 
// Setting up PORT 
const PORT = process.env.PORT || 3000; 
  
// Creating http Server 
var httpServer = http.createServer( 
       function(request, response) { 
        console.log(request.headers);
  // Setting up Headers 
  response.setHeader('Content-Type', 'text/html'); 
  response.write("<html><head><title>Premier serveur</title></head><body><p>salut</p></body></html>");
  response.end(); 
}); 
  
// Listening to http Server 
httpServer.listen(PORT, () => { 
    console.log("Server is running at port 3000..."); 
}); 
