//=========================================================================
// Description:
//    This program allows the user to
//    a) Generate a population file, including the birth & death years of each person.
//        The range of each person's life span is trimmed to be between 1900 and 2000, inclusively.
//    b) Process the file and report the year that the maximum number of people were alive.
//       If the maximum occurs in multile years, all years will be reported.
//
// Usage: SGI_WhosAlive populationFile [sizeOfPopulationToGenerate]
//    Where
//       'populationFile'             is the file to read from or write to,
//       'sizeOfPopulationToGenerate' is an integer specifying the number of records to generate for the file.
//    If no population size is specified, simply read and process the populationFile.
//=========================================================================


#include <iostream>        // for cout
#include <fstream>         // for ofstream,ifstream
#include <sstream>         // for stringstream
#include <time.h>          // for rand() seed
#include <assert.h>        // for assert
#include <vector>          // for vector
#include <list>            // for list
#include <memory>          // for weak_ptr
#include <algorithm>       // for min/max
using namespace std;

// Range, in years, of the population
#define RANGE_YEAR_END (2000)
#define RANGE_YEAR_BEG (1900)

// Semi-Realistic Life Span constants
#define MAX_AGE (130)
#define RANGE_YEAR_MIN   (RANGE_YEAR_BEG-MAX_AGE+1)
#define RANGE_AGEAVG_END (90)      // Average person lives to somewhere in this range
#define RANGE_AGEAVG_BEG (60)
#define RANGE_AGEOUT_END (MAX_AGE) // Outliers live to somewhere in this range (overlaps Average range)
#define RANGE_AGEOUT_BEG (40)
#define RANGE_AGEMID_END (30)      // The invincible years (new drivers, drinking, ...) deaths happen somewhere in this range
#define RANGE_AGEMID_BEG (16)
#define RANGE_AGENEW_END (1)       // short-lived NewBorns live to somewhere in this range
#define RANGE_AGENEW_BEG (0)
#define RANGE_AGEBAD_END (RANGE_AGEOUT_END)    // BadLuck/Accidents can happen any time (overlaps all ranges)
#define RANGE_AGEBAD_BEG (RANGE_AGENEW_BEG)

//--------------------------------------------------------------------------
// Name: struct _vitalStats()
// Desc: container for personal info (name, years of birth & death)
//--------------------------------------------------------------------------
typedef struct _vitalStats
{
public:
    _vitalStats(string first, string last, int yrBirth, int yrDeath)
    {
        _first   = first;
        _last    = last;
        _yrBirth = yrBirth;
        _yrDeath = yrDeath;
        if (yrBirth > yrDeath)
            throw false;
    }

    // accessors
    string &firstName() { return _first;   }
    string &lastName()  { return _last;    }
    int     birthYear() { return _yrBirth; }
    int     deathYear() { return _yrDeath; }
private:
    string _first;   // first name
    string _last;    // last  name
    int    _yrBirth; // RANGE_YEAR_BEG 'trimmed' year of birth
    int    _yrDeath; // RANGE_YEAR_END 'trimmed' year of death
} vitalStats_t;

//=========================================================================
// Name:    class argsAndErrs
// Desc:
//          object for
//          * accepting, sanity checking, and maintaining command line arguments
//          * reporting errors
//          * providing accessors to command line arguments and their derivatives
//=========================================================================
class argsAndErrs
{
public:
    argsAndErrs() : _sizeOfPopulation(-1) {}
public:
    enum { eCmdLnArg_AppPath, eCmdLnArg_PopFile, eCmdLnArg_PopSize };
    bool initWithArgs(int argc, const char * argv[], string &strErr);
    void reportErr(string strErr="");
    bool needData() { return (_sizeOfPopulation==-1) ? false : true; }
    friend class populationInfo; // needs access to protected functions
protected:
    void         addCmdLnArgsToErr(stringstream &ss);
    long long    populationSize() { return _sizeOfPopulation; }
    const string populationFile() { return _filePopulation; }
    
private:
    vector<string> _args;               // command line args
    string         _filePopulation;     // CLA[1] - population file to use
    long long      _sizeOfPopulation;   // CLA[2] - optional size of popuation -> generates file
};
typedef argsAndErrs argsAndErrs_t;

//=========================================================================
// Name:    class populationInfo
// Desc:
//          object for processing population-relevant command line arguments or commands.
//          Includes:
//          * generateVitalStats()    - generates a semi-realistice population data set
//          * findMaxPopulationYear() - finds and outputs the year(s) that had the most people alive
//=========================================================================
class populationInfo
{
public:
    populationInfo(weak_ptr<argsAndErrs_t>  wpFb) : _delim(';') { _wpFb = wpFb; }
    void generateVitalStats();
    void findMaxPopulationYear();
private:
    vector<string> deliminatedStringToTokens(const string &inpStr);
private:
    weak_ptr<argsAndErrs_t> _wpFb;      // hasa argsAndErrs_t
    const char              _delim;     // intra-record delimiter used to generate and parse data set info.
};

//--------------------------------------------------------------------------
// Name: initWithArgs()
// Desc:
//      * Accept input stings (putting them into m_args list)
//      * population file
//      * optional size of population (will generate a file)
// Params:
//       argc   - number of arguments
//       argv   - array of argument strings
//       strErr - if error, message to output
// Returns:
//      false if success; true if error
//--------------------------------------------------------------------------
bool argsAndErrs::initWithArgs(int argc, const char * argv[], string &strErr)
{
    // Accept input stings (putting them into m_args list)
    for (int ix = eCmdLnArg_AppPath; ix < argc; ix++)
    {
        _args.push_back(argv[ix]);
    }

    bool fDoBreak = false;
    do
    {
        for (int ixCmdLnArg = eCmdLnArg_PopFile; ixCmdLnArg < argc; ixCmdLnArg++)
        {
            switch (ixCmdLnArg)
            {
                case eCmdLnArg_PopFile:
                    _filePopulation = argv[eCmdLnArg_PopFile];
                    break;
                case eCmdLnArg_PopSize:
                    if (argc == eCmdLnArg_PopSize+1)
                    {
                        _sizeOfPopulation = 0;
                        try
                        {
                            _sizeOfPopulation = stoll(argv[eCmdLnArg_PopSize]);
                        }
                        catch (...)
                        {
                            stringstream ss;
                            ss <<  "    Problem with argment[" << ixCmdLnArg << "]." << endl;
                            ss <<  "        '"<<argv[ixCmdLnArg]<<"'. Needs to be a valid integer, specifying the desired population size." << endl;
                            addCmdLnArgsToErr(ss);
                            strErr = ss.str();
                            fDoBreak = true;
                        }
                    }
                    break;
                default:
                    {
                        stringstream ss;
                        ss <<  "    Too many CmdLineArgs." << endl;
                        addCmdLnArgsToErr(ss);
                        strErr = ss.str();
                        fDoBreak = true;
                    }
                    break;
            }
        }
        if (fDoBreak)
            break;
        
        if (_sizeOfPopulation == -1)
        {
            // ensure the file exists
            ifstream file;
            file.open (_filePopulation.c_str());
            if (file.is_open())
                file.close();
            else if (argc > eCmdLnArg_PopFile)
            {
                stringstream ss;
                ss <<  "    Problem with argment[" << eCmdLnArg_PopFile << "]." << endl;
                ss <<  "        '"<<argv[eCmdLnArg_PopFile]<<"' does not exist." << endl;
                addCmdLnArgsToErr(ss);
                strErr = ss.str();
                fDoBreak = true;
            }
            else
            {
                stringstream ss;
                ss <<  "    This program requires at least 1 argment (name of population file)" << endl;
                addCmdLnArgsToErr(ss);
                strErr = ss.str();
                fDoBreak = true;
            }
        }
    } while (false);
    
    
    return fDoBreak;
} //initWithArgs()

//--------------------------------------------------------------------------
// Name: reportErr()
// Desc:
//      output out an error message to std out
//      * Error Message (if any) is prefixed with an application usage message
// Params:
//       strErr - error message to output
// Returns:
//      void
//--------------------------------------------------------------------------
void argsAndErrs::reportErr(string strErr/*""*/)
{
    #define PATH_U_DELIM ('/')
    #define PATH_W_DELIM ('\\')
    #define PATH_C_DELIM (':')
    
    // trim application path
    static char PATH_DELIMS[] = { PATH_U_DELIM, PATH_W_DELIM, PATH_C_DELIM };
    string appName = _args.size() ? *_args.begin() : "<NoAppName>";
    size_t posPathDelim = appName.find_last_of(PATH_DELIMS);
    if(string::npos != posPathDelim)
    {
        appName = appName.substr(posPathDelim+1);
    }
    
    cerr << endl;
    cerr << "Description:" << endl;
    cerr << "   This program allows the user to " << endl;
    cerr << "   a) Generate a population file, including the birth & death years of each person." << endl;
    cerr << "   b) Process the file and report the year that the maximum number of people were alive." << endl;
    cerr << "      If the maximum occurs in multile years, all years will be reported." << endl;
    cerr << endl;
    cerr << "Usage: " << appName << " populationFile [sizeOfPopulationToGenerate]" << endl;
    cerr << "   Where " << endl;
    cerr << "      'populationFile'             is the file to read from or write to," << endl;
    cerr << "      'sizeOfPopulationToGenerate' is an integer specifying the number of records to generate for the file." << endl;
    cerr << "   If no population size is specified, this program will simply read and process the populationFile."  << endl;
    cerr << endl;
    if (strErr.length())
    {
        cerr << "Error: " << endl;
        cerr << strErr;
    }
} // reportErr()


//--------------------------------------------------------------------------
// Name: addCmdLnArgsToErr()
// Desc:
//      add the (post application name) Command Line Args to the current stream
// Params:
//       ss - output string stream to append to
// Returns:
//      void
//--------------------------------------------------------------------------
void argsAndErrs::addCmdLnArgsToErr(stringstream &ss)
{
    size_t ix=eCmdLnArg_AppPath;
    for (auto itCmdLnArg = _args.begin(); itCmdLnArg != _args.end(); ++itCmdLnArg)
        ss <<  "        param[" << ix++ << "] = " << *itCmdLnArg << endl;
} // addCmdLnArgsToErr()




//--------------------------------------------------------------------------
// Name: generateVitalStats()
// Desc:
//       Generate semi-realistic birth death years for people living in the desired time frame (RANGE_YEAR_BEG to RANGE_YEAR_END)
//       NOTE: this will generate births before the RANGE_YEAR_BEG and deaths after RANGE_YEAR_END for semi-realistic life spans,
//             then it will clip the ages to the desired time frame.
//             This generates a relatively FLAT population curve of the range.
//             Had we set RANGE_YEAR_MIN to RANGE_YEAR_BEG, we would get an upward ramping curve.
//             Had we only only allowed people who died in the range to be counted, we could get a downward ramping curve.
//             Had we only only allowed people who were born and died in the range to be counted, we could get a bell curve maximize around 1950.
// Params:
//       <none>
// Returns:
//      void
//--------------------------------------------------------------------------
void populationInfo::generateVitalStats()
{
    time_t secSinceEpoc; time(&secSinceEpoc);
    srand(secSinceEpoc);
    
    vector<vitalStats_t> vPopulationStats;
    
    do
    {
        shared_ptr<argsAndErrs_t> pFB = _wpFb.lock();
        if (pFB==nullptr || pFB.get()==nullptr)
            break;
        
        long long populationSize = pFB->populationSize();
        cout << "generating "<< populationSize << " records..." << endl;
    
        while (populationSize)
        {
            // Make some babies
            int yrBirth  = (rand() % (RANGE_YEAR_END  - RANGE_YEAR_MIN))  + RANGE_YEAR_MIN;
            
            // For whom does the bell toll?
            int inpBias  =  rand() % 100;
            int yrAlive  = (inpBias > 60) ? (rand() % (RANGE_AGEAVG_END  - RANGE_AGEAVG_BEG))  + RANGE_AGEAVG_BEG
                         : (inpBias > 30) ? (rand() % (RANGE_AGEOUT_END  - RANGE_AGEOUT_BEG))  + RANGE_AGEOUT_BEG
                         : (inpBias > 20) ? (rand() % (RANGE_AGENEW_END  - RANGE_AGENEW_BEG))  + RANGE_AGENEW_BEG
                         : (inpBias > 10) ? (rand() % (RANGE_AGEMID_END  - RANGE_AGEMID_BEG))  + RANGE_AGEMID_BEG
                         :                  (rand() % (RANGE_AGEBAD_END  - RANGE_AGEBAD_BEG))  + RANGE_AGEBAD_BEG
                         ;
            int yrDeath = yrBirth + yrAlive;
            
            if (yrDeath > RANGE_YEAR_BEG)
            {
                // Could have used a name generation site like: http://listofrandomnames.com/, but since names are not relevant to the problem...
                // Obfuscate/Redact 'real' names for privacy protection ;)
                vPopulationStats.push_back(_vitalStats("<Name Redacted>", "<For Privacy>", max(RANGE_YEAR_BEG, yrBirth), min(RANGE_YEAR_END,yrDeath)));
                populationSize--;
            }
        }

        cout << "generated "<< vPopulationStats.size() << " records." << endl;
        cout << "adding  "  << vPopulationStats.size() << " records to file '" << pFB->populationFile().c_str() << "'" << endl;
        
        ofstream outStream;
        outStream.open(pFB->populationFile().c_str());
        if (outStream.is_open())
        {
            for (auto person : vPopulationStats)
            {
                outStream << person.firstName() << _delim // eFileTokenFName
                          << person.lastName()  << _delim // eFileTokenLName
                          << person.birthYear() << _delim // eFileTokenBYear
                          << person.deathYear() << endl;  // eFileTokenDYear
            }
            outStream.close();
            cout << "added  "  << vPopulationStats.size() << " records to file '" << pFB->populationFile().c_str() << "'" << endl;
        }
        else
        {
            {
                stringstream ss;
                ss <<  "    Unable to open specified file,'" << pFB->populationFile().c_str() << "', for write." << endl;
                pFB->addCmdLnArgsToErr(ss);
                string strErr = ss.str();
                pFB.get()->reportErr(strErr);
            }
        }
    } while (false);
}

//--------------------------------------------------------------------------
// Name: deliminatedStringToTokens()
// Desc:
//        returns a vector of substrings that were delimited by the specified token
// Params:
//       inpStr - a string, delimited by _delim (in generateVitalStats())
// Returns:
//      vector of substrings
//--------------------------------------------------------------------------
vector<string> populationInfo::deliminatedStringToTokens(const string &inpStr)
{
    vector<string> tokens;

    do
    {
        auto itToken=inpStr.begin();
        for (auto itInp=itToken; itInp!=inpStr.end(); ++itInp)
        {
            if (*itInp == _delim)
            {
                tokens.push_back(string(itToken,itInp));
                itToken = itInp+1;
            }
        }
        tokens.push_back(string(itToken,inpStr.end()));

    } while (false);

    return tokens;
}

//--------------------------------------------------------------------------
// Name: findMaxPopulationYear()
// Desc:
//        Process the file and report the year that the maximum number of people were alive.
//        If the maximum occurs in multile years, all years will be reported
// Params:
//       <none>
// Returns:
//      void
//--------------------------------------------------------------------------
void populationInfo::findMaxPopulationYear()
{
    vector<vitalStats_t> vPopulationStats;
    
    bool fDoBreak = false;
    do
    {
        shared_ptr<argsAndErrs_t> pFB = _wpFb.lock();
        if (pFB==nullptr || pFB.get()==nullptr)
            break;
        
        vector<long long>airBreathers(RANGE_YEAR_END-RANGE_YEAR_BEG+1, 0);
        list<long long> MaxYears;

        long long maxAlive = 0;
         cout << "reading records from file '" << pFB->populationFile().c_str() << "'" << endl;
   
        ifstream inpStream;
        inpStream.open(pFB->populationFile().c_str());
        if (inpStream.is_open())
        {
            string strDelimitedLine;
            vector<vitalStats_t> vPopulationStats;
            
            long long ixRecord=0;
            int yrBirth;
            int yrDeath;
            while (getline(inpStream, strDelimitedLine))
            {
                ixRecord++;
                enum { eFileTokenFName, eFileTokenLName, eFileTokenBYear, eFileTokenDYear };
                vector<string> tokens = deliminatedStringToTokens(strDelimitedLine);
                
                        try
                        {
                            yrBirth = stoi(tokens[eFileTokenBYear]);
                            yrDeath = stoi(tokens[eFileTokenDYear]);
                        }
                        catch (...)
                        {
                            stringstream ss;
                            ss <<  "    File corrupted at record " << ixRecord << "." << endl;
                            ss <<  "    Expecting '"<<tokens[eFileTokenBYear]<<"' to be a valid integer" << endl;
                            ss <<  "    Expecting '"<<tokens[eFileTokenDYear]<<"' to be a valid integer" << endl;
                            pFB->addCmdLnArgsToErr(ss);
                            string strErr = ss.str();
                            pFB.get()->reportErr(strErr);
                            fDoBreak = true;
                            break;
                        }
                        // _vitalStats stats(tokens[eFileTokenFName],tokens[eFileTokenLName], yrBirth, yrDeath);

                // Add this person's alive years to the population count for each year in range
                int ixAlive  = yrBirth - RANGE_YEAR_BEG;
                int lastYear = yrDeath - RANGE_YEAR_BEG;
                for (; ixAlive <= lastYear; ixAlive++)
                {
                    long long cntAlive = ++airBreathers[ixAlive]; // this person is alive this year

                    if (cntAlive > maxAlive)
                    {
                        // This year now has the largest population
                        MaxYears.clear();
                        MaxYears.push_back(ixAlive);
                        maxAlive = cntAlive;
                    }
                    else if (cntAlive == maxAlive)
                    {
                        MaxYears.push_back(ixAlive);
                    }
                } // for each year that the person is alive


// Simple graph to show population
//printf("yrBirth:%d   yrDeath:%d\n", yrBirth, yrDeath);
//for (ixAlive=0; ixAlive <= 100; ixAlive++)
//{
//    printf("[%3d]", ixAlive);
//    for (int ix=0; ix < airBreathers[ixAlive]; ix++)
//    {
//        if (ix%10 == 1)
//            printf("*");
//    }
//    printf("\n");
//}



            } // while() there are more people to read in
            
            if (fDoBreak)
                break;
            
            int ixYear=0;
            if (MaxYears.size() == 0)
            {
                cout << "There were no records to process in file '" << pFB->populationFile().c_str() << "'" << endl;
                break;
            }
            
            cout << endl;
            if (MaxYears.size() == 1)
                cout << "The year ";
            else
            {
                cout << "The " << MaxYears.size() << " years ";
            }
            cout << "with the the highest population (" << maxAlive<< ") "<< ((MaxYears.size() == 1) ? "was:" : "were:") << endl;
            cout << "{ "  ;
            for (auto itMaxYr=MaxYears.begin(); itMaxYr!=MaxYears.end(); ++itMaxYr)
            {
                cout << (*itMaxYr + RANGE_YEAR_BEG)  << ((++ixYear == MaxYears.size()) ? " }\n\n" : ", ");
            }
        }
        else
        {
            {
                stringstream ss;
                ss <<  "    Unable to open specified file,'" << pFB->populationFile().c_str() << "', for write." << endl;
                pFB->addCmdLnArgsToErr(ss);
                string strErr = ss.str();
                pFB.get()->reportErr(strErr);
            }
        }
    } while (false);
}

int main(int argc, const char * argv[])
{
    bool fDoBreak = false;
    do
    {
        string strErr;
        argsAndErrs fb;
    
        //--------------------------------------------------------------------------------------------
        // Input and sanity check command line args
        //--------------------------------------------------------------------------------------------
        if (( fDoBreak = fb.initWithArgs(argc,  argv, strErr) ))
        {
            fb.reportErr(strErr);
            break;
        }
        
        //--------------------------------------------------------------------------------------------
        // Do the work - Find combinations of orginal message after hidden message chars are removed
        //--------------------------------------------------------------------------------------------
        shared_ptr<argsAndErrs_t>spFB = make_shared<argsAndErrs_t>(fb);
        
        populationInfo myPeeps(spFB);
        if (fb.needData())
        {
            // Make some babies and see how long they last
            myPeeps.generateVitalStats();
        }
        //else
        {
            // Muster Call
            myPeeps.findMaxPopulationYear();
        }
    } while (false);
    
    return 0;
}
