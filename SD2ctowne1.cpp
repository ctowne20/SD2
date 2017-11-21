///////////////////////////////////////////////////////////////////////////
//
// Module Name:   LOCcounter.c
//
// Module Description:
// This program is designed to count the number of lines of code in a file
//   and display them onto the screen. If the file does not exist it will just
//   end the execution of the program without error.
// This is an edit from the original program that has just the basic
//   components and is platform independent.
//
///////////////////////////////////////////////////////////////////////////
//
// Author:          Nathan McCollister, Benjamin Sweet, Carrieann Towne
// Revision:        1.3
// Creation Date:   28-May-2014
// Edit Date:       21_Nov-2017
//
///////////////////////////////////////////////////////////////////////////
//
// Revision History:
//
// Revision: 1.3	21-Nov-2017	  C. Towne
// * Added functionality to find functions and determine how many lines of
//	 code they have
//
// Revision: 1.2    21-Aug-2016   B. Sweet
// * Resolved error in which a block comment containing '/' that is not part
//   of the end comment delimiter would be counted as LOC.
//
// Revision: 1.1    06-Feb-2016   B. Sweet
// * Revised where and how iTotalLOC is incremented to provide more obvious
//   "state" behavior.
// * Resolved error in which a line containing only '*' would be counted as LOC.
//
// Revision: 1.0    28-May-2014   Nathan McCollister
// * Original version.
//  
///////////////////////////////////////////////////////////////////////////

#include "stdafx.h"		//Necessary include statement for the compiler
#include <iostream>
#include <fstream>
#include <string>
#define MAX_NAME_SIZE 20
#define MAX_FUNCTIONS 10

//  using namespace std;               // std:: is used in lieu of using namespace std

typedef struct
{
	int funcLOC = 0;
	char funcName[MAX_NAME_SIZE] = "";
}FUNCDATA; 

int countFuncLOC(std::ifstream &fileName, FUNCDATA*funcDataPtr)
{
	bool bContinueThisLine = true;     // a flag to determine if the loop should continue searching the current line.
	bool bBlockComment = false;		   // a flag to determine if the program is currently inside of a block comment
	bool bIsLOC = false;               // a flag to indicate that the current line IS a Line of Code
	bool bIsFunc = false;			   // a flag to indicate that the current line is in a function
	int iTotalLOC = 0;                 // count of Total LOC found in the file
	int iIsBracket = 0;				   // count of brackets for use in function determination
	int numFunc = 0;				   // number of functions (used for placement in FUNCDATA table)
	std::string tmpString, sTemp;	   // temporary use strings used to determine the function name

	//
	// A Loop to scan file line-by-line until end of file (EOF)
	//
	while (!fileName.eof())
	{
		getline(fileName, tmpString);
		bContinueThisLine = true;      // reset to true at the beginning of each new line
									   //
									   // A loop to scan each line character-by-character
									   //
		for (int i = 0; (i < tmpString.length()) && (tmpString[i] != '\n') && (bContinueThisLine == true); i++)        // a loop to scan each line character by character
		{
			switch (tmpString[i])
			{
			case '/':                // if line contains '/' then check if it is a line comment, block comment, or neither
				if (bBlockComment == false)  // If NOT in a block comment, check for beginning of comment
				{
					if (i < tmpString.length() - 1)
					{
						if (tmpString[i + 1] == '/')            // This begins a C++ line comment
						{
							bContinueThisLine = false;          //   Get the next line
						}
						else if (tmpString[i + 1] == '*')       // This begins a C block comment
						{
							bBlockComment = true;
							i++;                                // Already evaluated next character so don't evaluate it again
						}
						else                                    // '/' that is NOT part of a comment delimiter indicates this IS a Line of Code
						{
							bIsLOC = true;
						}
					}
				}
				else // In a Block Comment
				{
					// Ignore the '/' character.  Logic is looking ONLY for the '*' character.  
				}
				break;
			case '*':                // if currently in a block comment it checks to see if it is closing the comment
				if (bBlockComment == false)                 // '*' that is NOT part of a block comment delimiter indicates a Line of Code
				{
					bIsLOC = true;
				}
				else if (i < tmpString.length() - 1)
				{
					if (tmpString[i + 1] == '/')            // Check for end of Block Comment
					{
						bBlockComment = false;              // End of Block Comment
						i++;                                // Already evaluated next character so don't evaluate it again
					}
				}
				break;
			default:                 // all other non-whitespace characters indicate a LOC assuming not in a block comment
				if ((bBlockComment == true) || (isspace(tmpString[i])))
				{
					// Skip to the next character or line
				}
				else
				{
					bIsLOC = true;
				}
				break;
			}
		}   // Scan line character by character

		if (bIsLOC == true)            // If flag indicates current line is LOC
		{
			++iTotalLOC;               // Tally LOC
			
 			if (tmpString.find("{") != std::string::npos)
				bIsFunc = true;

			if (bIsFunc == true)
			{
				funcDataPtr->funcLOC++; 		   // Tally LOC for the function
				
				if (tmpString.find("{") != std::string::npos)
					//if (iIsBracket != 0)
					iIsBracket++;  // Tally number of brackets
				else if (tmpString.find("}") != std::string::npos)
					iIsBracket--;	   // Tally number of brackets

				if (iIsBracket == 0)	// When the line is no longer in a bracket, it is out of the function!
					bIsFunc = false;
			}

			if (tmpString.find(";") == std::string::npos && tmpString.find("#") == std::string::npos && tmpString.find("(") != std::string::npos && tmpString.find(")") != std::string::npos)
			{
				auto begin = tmpString.find_first_of("(");
				auto end = tmpString.find_last_of(")");
				if (std::string::npos != begin && std::string::npos != end && begin <= end)
				{
					tmpString = tmpString.erase(begin, tmpString.length() - begin);		//remove the parameters

					auto space = tmpString.find_last_of(" ");

					if (tmpString.length() - space == 1)	//if there is a space between the parameters and the function name, remove it
						sTemp = tmpString.erase(space, tmpString.length() - space);	
					
					if (sTemp.find(" ") == std::string::npos) //this is to check if the line is a loop rather than a function declaration
					{
						bIsFunc = false;
						break;
					}

					space = tmpString.find_last_of(" ");
					sTemp = tmpString.substr(space + 1, tmpString.length() - space);	//the final function name is the string between the parameters and the definition clarifiers
					
					for (int i = 0; i < sTemp.length(); i++)
						funcDataPtr->funcName[i] = sTemp[i];
				}
				funcDataPtr->funcLOC++;			  // Tally LOC in the function

				if (tmpString.find("{") != std::string::npos && tmpString.find("}") == std::string::npos)
					iIsBracket++;	      // Tally brackets

				bIsFunc = true;		  // In a function!
				numFunc++;
			}
			bIsLOC = false;       // Clear the flag for the next iteration
		}

	}  // Loop through file line-by-line until finding EOF

	return iTotalLOC;
}

void main()
{
	FUNCDATA funcDataTable; // [MAX_FUNCTIONS];
	FUNCDATA *funcDataPtr;
	std::string sFileName;
	std::ifstream fin;

	funcDataPtr = &funcDataTable;

	std::cout << "Please enter the file name:   ";
	std::cin >> sFileName;
	fin.open(sFileName.c_str(), std::ios::in);

	if (fin.is_open())
	{

		std::cout << "File Name: " << sFileName << "\nFile LOC:     " << countFuncLOC(fin, funcDataPtr) << "\n\n" << "Function(s)";
		
		int i = 0;
		while (funcDataPtr != NULL)
		{
			std::cout << "\n" << funcDataPtr->funcName << "- " << funcDataPtr->funcLOC;
			i++;
		}
		fin.close();
	}
	else // file does not exist
	{
		std::cout << "filename: " << sFileName << " - File does not exist.\n\n";
	}

	std::cout << "Press \"Enter\" key to continue...\n";
	std::cin.ignore();
	std::getchar();

	return;
}
