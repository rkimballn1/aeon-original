/*
 Copyright 2016 Nervana Systems Inc.
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/
#pragma once

#include <sys/stat.h>
#include <libgen.h>

#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include <opencv2/core/core.hpp>

#include "media.hpp"
#include "codec.hpp"
#include "specgram.hpp"
#include "noise_clips.hpp"
#include "audio_params.hpp"

using std::vector;
using std::ifstream;
using cv::Mat;

class Audio : public Media {
public:
    Audio(AudioParams *params, int randomSeed);
    virtual ~Audio();

    void dumpToBin(char* filename, RawMedia* audio, int idx);
    void transform(char* item, int itemSize, char* buf, int bufSize, int* meta);
    RawMedia* decode(char* item, int itemSize);
    void newTransform(RawMedia* raw, char* buf, int bufSize, int* meta);
    void ingest(char** dataBuf, int* dataBufLen, int* dataLen);

private:
    AudioParams*                _params;
    Codec*                      _codec;
    Specgram*                   _specgram;
    NoiseClips*                 _noiseClips;
    bool                        _loadedNoise;
    cv::RNG                     _rng;
};
