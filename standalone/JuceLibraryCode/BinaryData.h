/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#ifndef BINARYDATA_H_122324817_INCLUDED
#define BINARYDATA_H_122324817_INCLUDED

namespace BinaryData
{
    extern const char*   audfile_png;
    const int            audfile_pngSize = 1512;

    extern const char*   audfileOn_png;
    const int            audfileOn_pngSize = 3375;

    extern const char*   back_png;
    const int            back_pngSize = 1082;

    extern const char*   backOn_png;
    const int            backOn_pngSize = 2328;

    extern const char*   forward_png;
    const int            forward_pngSize = 1071;

    extern const char*   forwardOn_png;
    const int            forwardOn_pngSize = 2324;

    extern const char*   incback_png;
    const int            incback_pngSize = 1323;

    extern const char*   incbackOn_png;
    const int            incbackOn_pngSize = 3490;

    extern const char*   incforward_png;
    const int            incforward_pngSize = 1396;

    extern const char*   incforwardOn_png;
    const int            incforwardOn_pngSize = 3277;

    extern const char*   mic_png;
    const int            mic_pngSize = 2734;

    extern const char*   micOn_png;
    const int            micOn_pngSize = 11077;

    extern const char*   pause_png;
    const int            pause_pngSize = 891;

    extern const char*   pauseOn_png;
    const int            pauseOn_pngSize = 1861;

    extern const char*   play_png;
    const int            play_pngSize = 1112;

    extern const char*   playOn_png;
    const int            playOn_pngSize = 2229;

    extern const char*   save_png;
    const int            save_pngSize = 995;

    extern const char*   saveOn_png;
    const int            saveOn_pngSize = 1201;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Number of elements in the namedResourceList array.
    const int namedResourceListSize = 18;

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes) throw();
}

#endif
