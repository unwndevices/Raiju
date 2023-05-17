#include <Arduino.h>
#include <utils/dsp.h>
#include <utils/OnePole.h>

using namespace daisysp;

class HardClipper
{

    // THIS CLASS IMPLEMENTS AN ANTIALIASED HARD CLIPPING FUNCTION.
    // THIS CLASS USES THE FIRST-ORDER ANTIDERIVATIVE METHOD.

private:
    float output = 0.0;

    float xn1 = 0.0;
    float Fn = 0.0;
    float Fn1 = 0.0;

    const float thresh = 10.0e-2;
    const float oneTwelfth = 1.0 / 12.0;

public:
    HardClipper() {}
    ~HardClipper() {}

    void Process(float input)
    {
        output = antialiasedHardClipN1(input);
    }

    float signum(float x)
    {
        return (x > 0.0f) ? 1.0f : ((x < 0.0f) ? -1.0f : 0.0f);
    }

    float hardClipN0(float x)
    {
        // Hard clipping function
        return 0.5f * (signum(x + 1.0f) * (x + 1.0f) - signum(x - 1.0f) * (x - 1.0f));
    }

    float hardClipN1(float x)
    {
        // First antiderivative of hardClipN0
        return 0.25f * (signum(x + 1.0f) * (x + 1.0f) * (x + 1.0f) - signum(x - 1.0f) * (x - 1.0f) * (x - 1.0f) - 2.0f);
    }

    float hardClipN2(float x)
    {
        // second antiderivative of hardClipN0
        return oneTwelfth * (signum(x + 1.0f) * (x + 1.0f) * (x + 1.0f) * (x + 1.0f) - signum(x - 1.0f) * (x - 1.0f) * (x - 1.0f) * (x - 1.0f) - 6.0f * x);
    }

    float antialiasedHardClipN1(float x)
    {

        // Hard clipping with 1st-order antialiasing
        Fn = hardClipN1(x);
        float tmp = 0.0;
        if (std::abs(x - xn1) < thresh)
        {
            tmp = hardClipN0(0.5f * (x + xn1));
        }
        else
        {
            tmp = (Fn - Fn1) / (x - xn1);
        }

        // Update states
        xn1 = x;
        Fn1 = Fn;

        return tmp;
    }

    float getClippedOutput()
    {
        return output;
    }
};

class Wavefolder
{
private:
    float output = 0.0;

    // Antialiasing state variables
    float xn1 = 0.0;
    float xn2 = 0.0;
    float Fn = 0.0;
    float Fn1 = 0.0;
    float Gn = 0.0;
    float Gn1 = 0.0;

    // Ill-conditioning threshold
    float thresh = 0.1f;

    const float oneSixth = 1.0 / 6.0;

    HardClipper hardClipper;

public:
    Wavefolder() {}
    ~Wavefolder() {}

    inline void setThreshold(float threshold) { thresh = threshold; }
    float Process(float input)
    {
        output = antialiasedFoldN2(input);
        return output;
    }

    float foldFunctionN0(float x)
    {
        // Folding function
        return (2.0f * hardClipper.hardClipN0(x) - x);
    }

    float foldFunctionN1(float x)
    {
        // First antiderivative of the folding function
        return (2.0f * hardClipper.hardClipN1(x) - 0.5f * x * x);
    }

    float foldFunctionN2(float x)
    {
        // Second antiderivative of the folding function
        return (2.0f * hardClipper.hardClipN2(x) - oneSixth * (x * x * x));
    }

    float antialiasedFoldN1(float x)
    {

        // Folding with 1st-order antialiasing (not recommended)
        Fn = foldFunctionN1(x);
        float tmp = 0.0;
        if (std::abs(x - xn1) < thresh)
        {
            tmp = foldFunctionN0(0.5f * (x + xn1));
        }
        else
        {
            tmp = (Fn - Fn1) / (x - xn1);
        }

        // Update states
        xn1 = x;
        Fn1 = Fn;

        return tmp;
    }

    float antialiasedFoldN2(float x)
    {

        // Folding with 2nd-order antialiasing
        Fn = foldFunctionN2(x);
        float tmp = 0.0;
        if (std::abs(x - xn1) < thresh)
        {
            // First-order escape rule
            Gn = foldFunctionN1(0.5f * (x + xn1));
        }
        else
        {
            Gn = (Fn - Fn1) / (x - xn1);
        }

        if (std::abs(x - xn2) < thresh)
        {
            // Second-order escape
            float delta = 0.5f * (x - 2.0f * xn1 + xn2);
            if (std::abs(delta) < thresh)
            {
                tmp = foldFunctionN0(0.25f * (x + 2.0f * xn1 + xn2));
            }
            else
            {
                float tmp1 = foldFunctionN1(0.5f * (x + xn2));
                float tmp2 = foldFunctionN2(0.5f * (x + xn2));
                tmp = (2.0f / delta) * (tmp1 + (Fn1 - tmp2) / delta);
            }
        }
        else
        {
            tmp = 2.0f * (Gn - Gn1) / (x - xn2);
        }

        // Update state variables
        Fn1 = Fn;
        Gn1 = Gn;
        xn2 = xn1;
        xn1 = x;

        return tmp;
    }

    float getFoldedOutput()
    {
        return output;
    }
};

const int nFolders = 3;

class b259
{
public:
    void Init(float sr)
    {
        lpf.Init(1500.0f, sr);
        gainLevel = 0.7f;
        symmLevel = 0.0f;
    }
    inline void SetTone(float tone) { lpf.setFilter(fclamp(tone, 600.0f, 6000.0f)); }
    inline void SetFold(float fold) { foldLevel = fclamp(fold, -10.0f, 10.0f); }
    inline void SetGain(float gain) { gainLevel = gain; }
    inline void SetSymm(float symm) { symmLevel = fclamp(symm, -10.0f, 10.0f); };
    inline float Process(float input)
    {
        output = input * foldLevel + symmLevel;
        for (int i = 0; i < nFolders; i++)
        {
            output = folder[i].Process(output);
        }
        output = SoftClip(lpf.Process(output));
        return output * gainLevel;
    }

private:
    Wavefolder folder[nFolders];
    // HardClipper clipper;
    OnePole lpf;
    float foldLevel, symmLevel, gainLevel, output = 0.0f;
};