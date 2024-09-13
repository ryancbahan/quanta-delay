#include <array>
#include <cmath>
#include <algorithm>

class TremoloManager
{
public:
    TremoloManager() : rate(2.0f), depth(0.5f), sampleRate(44100.0), phase(0.0f)
    {
        initializeSineLookup();
    }

    void prepare(double newSampleRate, int samplesPerBlock)
    {
        sampleRate = newSampleRate;
        updateIncrement();
        
        // Initialize filters
        for (int i = 0; i < 4; ++i)
        {
            filters[i].coeffs = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
            filters[i].state = {0.0f, 0.0f, 0.0f, 0.0f};
        }
        updateFilters();
    }

    void setRate(float newRate)
    {
        rate = newRate;
        updateIncrement();
    }

    void setDepth(float newDepth)
    {
        depth = std::clamp(newDepth, 0.0f, 1.0f);
    }

    void processBlock(float* leftChannel, float* rightChannel, int numSamples)
    {
        for (int i = 0; i < numSamples; i += 4)
        {
            processChunk(leftChannel + i, rightChannel + i, std::min(4, numSamples - i));
        }
    }

private:
    float rate;
    float depth;
    double sampleRate;
    float phase;
    float phaseIncrement;

    static constexpr size_t SINE_TABLE_SIZE = 1024;
    std::array<float, SINE_TABLE_SIZE> sineTable;

    struct FilterCoeffs
    {
        float a1, a2, b0, b1, b2;
    };

    struct FilterState
    {
        float x1, x2, y1, y2;
    };

    struct Filter
    {
        FilterCoeffs coeffs;
        FilterState state;
    };

    std::array<Filter, 4> filters; // LowL, HighL, LowR, HighR

    void initializeSineLookup()
    {
        for (size_t i = 0; i < SINE_TABLE_SIZE; ++i)
        {
            sineTable[i] = std::sin(2.0f * M_PI * i / SINE_TABLE_SIZE);
        }
    }

    float fastSin(float x)
    {
        x -= std::floor(x);
        return sineTable[static_cast<size_t>(x * SINE_TABLE_SIZE) % SINE_TABLE_SIZE];
    }

    void updateIncrement()
    {
        phaseIncrement = rate / static_cast<float>(sampleRate);
    }

    void updateFilters()
    {
        float w0 = 2.0f * M_PI * 500.0f / static_cast<float>(sampleRate);
        float cosw0 = std::cos(w0);
        float alpha = std::sin(w0) / (2.0f * 0.707f);

        // Lowpass filter coefficients
        float lpA0 = 1.0f + alpha;
        filters[0].coeffs.b0 = filters[2].coeffs.b0 = ((1.0f - cosw0) / 2.0f) / lpA0;
        filters[0].coeffs.b1 = filters[2].coeffs.b1 = (1.0f - cosw0) / lpA0;
        filters[0].coeffs.b2 = filters[2].coeffs.b2 = ((1.0f - cosw0) / 2.0f) / lpA0;
        filters[0].coeffs.a1 = filters[2].coeffs.a1 = (-2.0f * cosw0) / lpA0;
        filters[0].coeffs.a2 = filters[2].coeffs.a2 = (1.0f - alpha) / lpA0;

        // Highpass filter coefficients
        float hpA0 = 1.0f + alpha;
        filters[1].coeffs.b0 = filters[3].coeffs.b0 = ((1.0f + cosw0) / 2.0f) / hpA0;
        filters[1].coeffs.b1 = filters[3].coeffs.b1 = -(1.0f + cosw0) / hpA0;
        filters[1].coeffs.b2 = filters[3].coeffs.b2 = ((1.0f + cosw0) / 2.0f) / hpA0;
        filters[1].coeffs.a1 = filters[3].coeffs.a1 = (-2.0f * cosw0) / hpA0;
        filters[1].coeffs.a2 = filters[3].coeffs.a2 = (1.0f - alpha) / hpA0;
    }

    float processSample(float input, Filter& filter)
    {
        float output = filter.coeffs.b0 * input + filter.coeffs.b1 * filter.state.x1
                     + filter.coeffs.b2 * filter.state.x2 - filter.coeffs.a1 * filter.state.y1
                     - filter.coeffs.a2 * filter.state.y2;

        filter.state.x2 = filter.state.x1;
        filter.state.x1 = input;
        filter.state.y2 = filter.state.y1;
        filter.state.y1 = output;

        return output;
    }

    void processChunk(float* leftChannel, float* rightChannel, int chunkSize)
    {
        float lowLeft[4], highLeft[4], lowRight[4], highRight[4];

        // Split signals
        for (int i = 0; i < chunkSize; ++i)
        {
            lowLeft[i] = processSample(leftChannel[i], filters[0]);
            highLeft[i] = processSample(leftChannel[i], filters[1]);
            lowRight[i] = processSample(rightChannel[i], filters[2]);
            highRight[i] = processSample(rightChannel[i], filters[3]);
        }

        // Calculate modulation
        float modLow = (fastSin(phase) * 0.5f + 0.5f) * depth;
        float modHigh = (fastSin(phase + 0.25f) * 0.5f + 0.5f) * depth;

        // Apply modulation and recombine
        for (int i = 0; i < chunkSize; ++i)
        {
            leftChannel[i] = lowLeft[i] * (1.0f - modLow) + highLeft[i] * (1.0f - modHigh);
            rightChannel[i] = lowRight[i] * (1.0f - modLow) + highRight[i] * (1.0f - modHigh);
        }

        // Update phase
        phase += phaseIncrement * chunkSize;
        phase -= std::floor(phase);
    }
};
