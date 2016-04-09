The accompanying code is a solution to the following SGI programming problem:


   Problem 1:

	Description:

		Given a list of people with their birth and end years (all between 1900 and 2000), find the year with the most number of people alive.

	Code

		Solve using a language of your choice and dataset of your own creation.

	Submission

		Please upload your code, dataset, and example of the program’s output to Bit Bucket or Github. Please include any graphs or charts created by your program.



   Solution:
        Description:
		This program allows the user to
		   a) Generate a population file, including the birth & death years of each person.
			This dataset includes people who were born before 1900 or died after 2000,
			however, their lifespan is 'trimmed' to fit in the 1900 to 2000 year range.
		   b) Process the file and report the year that the maximum number of people were alive.
		      If the maximum occurs in multile years, all years will be reported.

		Usage: SGI_WhoIsAlive populationFile [sizeOfPopulationToGenerate]
	   	   Where
		      'populationFile'             is the file to read from or write to,
		      'sizeOfPopulationToGenerate' is an integer specifying the number of records to generate for the file.
	   					     If no population size is specified, this program will simply read and process
				    	             the populationFile.
               
 
	Tools:

		This code was written assuming a c++11 tool set.

	Build:

		This code may be built and run on windows or mac, using:

		Mac:  g++ -o3  -std=c++0x main.cpp -o  WhoIsAlive.app

		Win:  cl                  main.cpp  /FeWhoIsAlive.exe
	Run:

		1) This example will:

		    * generate a 1000 birth/death records,
 
		    * write them to a MyPeeps.txt file,
 
		    * read and parse the MyPeeps.txt file
,
		    * and determine the year(s) with the most people alive.


  
		    Mac: 
			./WhoIsAlive.app MyPeeps.txt 1000

		    Win: 
			./WhoIsAlive.exe MyPeeps.txt 1000



		    Two possible outputs might be:

		    a) The year with the the highest population (418) was:

		       { 1922 }

		    b) The 2 years with the the highest population (433) were:

		       { 1968, 1970 }




		2) This example will avoid data generation and will read/process an existing file:

		    * read and parse the MyPeeps.txt file

		    * and determine the year(s) with the most people alive.


  
		    Mac: 
			./WhoIsAlive.app MyPeeps.txt

		    Win: 
			./WhoIsAlive.exe MyPeeps.txt



		    The output will be the same as that when generated.
.