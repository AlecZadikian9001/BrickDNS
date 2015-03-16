BrickDNS
This is our Cal Hacks project, created using Xcode, written in C99 for use on POSIX systems.

Purpose of Project{

The purpose of this project is to make exact geo coordinates more memorable and easily transferrable between humans by giving each set of coordinates, down to 1/100000th of a degree for latitude and longitude, a unique and grammatically correct 4-word sentence (which we call a "BrickDNS sentence" in this document) to identify it. This is not only easier than dealing with number/address/city combinations but totally non-ambiguous and usable anywhere in the world. Street addresses sometimes have ambiguity or can be misinterpreted, unlike BrickDNS sentences. Different locations with in a building will have different BrickDNS sentences associated with them.

This is a proof of concept / prototype, but many existing services could implement it. For example, Apple's Siri could take BrickDNS sentences instead of street addresses when the user wants to navigate somewhere. On the non-tech side, people could very easily memorize exact locations and give each other their exact coordinates by verbal communication of BrickDNS sentences.

}

Details and Usage{

The server generates the words database, sql.db (must be in the working directory), based on the dictionary.txt file (must be in the working directory).

The server takes geo coordinates as (latitude, longitude) in degrees in decimal form, truncates them to 5 decimal places (precision of 1/100000th of a degree), and returns four English words that can be used to form a grammatically correct sentence. The server can also take these words and return back the original geo coordinates. In math terms, function from coordinates to words is one-to-one and onto. 

The web CGI script takes coordinates or words in the URL as parameters and communicates with the server (localhost) then returns the output to the client. URLs used to access the CGI script have the format "http://path/to/cgi/script.cgi?query", where "query" is replaced with:
If you want to get coordinates from words: "1word,word,word,word" example: "1miriest,faraday,hopingly,lurch"
If you want to get words from coordinates: "2latitude,longitude" (latitude and longitude given in decimal form) example: "234.1314392,-118.4433847" (for coordinates 34.1314392,-118.4433847)

A server is already up for this and has a CGI script: http://deepskate.com/BrickDNS/tester.cgi

Example words from coordinates for Soda Hall at Cal: http://deepskate.com/BrickDNS/tester.cgi?237.8756034,-122.2587475

Example coordinates from words for Soda Hall at Cal: http://deepskate.com/BrickDNS/tester.cgi?1celebrated,bouquet,aport,devoting

}

Known Bugs/Flaws{

- The dictionary used for this has a lot of obscure words. A dictionary only consisting of common words would still easily have enough words for this project, but we were unable to find one that also had grammatical categorization.

- There is very little error checking. This is a hack. A client cannot crash the server, but a false start of the server can lead to a crash. Make sure the sql.db file and the dictionary.txt file are in the working directory. Editing the dictionary.txt file in certain ways may result in a corrupted database, unpredictable behavior, or crashes.

- The server regenerates the words database every time it starts, and this is not a very optimized process.

- It is possible for a user to enter random words (that is, not taken from a server query for coordinates) and get coordinates from them, but the server maps those coordinates back to a different sentence (in violation of the bijective words-to-coordinates relationship). This only happens with sentences made up by the user. Any sentence generated by the server will map back to its original coordinates without a problem. This is caused by a bug in the database generation or accessing code, but the problem was discovered the day of the presentation at Cal Hacks and was not thoroughly investigated.

}
