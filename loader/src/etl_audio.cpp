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

#include "etl_audio.hpp"

using namespace std;
using namespace nervana;

shared_ptr<audio::params> audio::param_factory::make_params(std::shared_ptr<const decoded>)
{
    auto audio_stgs = shared_ptr<audio::params>(new audio::params());

    audio_stgs->add_noise             = _cfg.add_noise(_dre);
    audio_stgs->noise_index           = _cfg.noise_index(_dre);
    audio_stgs->noise_level           = _cfg.noise_level(_dre);
    audio_stgs->noise_offset_fraction = _cfg.noise_offset_fraction(_dre);
    audio_stgs->time_scale_fraction   = _cfg.time_scale_fraction(_dre);

    return audio_stgs;
}

std::shared_ptr<audio::decoded> audio::extractor::extract(const char* item, int itemSize)
{
    return make_shared<audio::decoded>(make_shared<wav_data>(item, (uint32_t) itemSize));
}

audio::transformer::transformer(const audio::config& config) :
    _cfg(config)
{
    specgram::create_window(_cfg.window_type, _cfg.frame_length_tn, _window);
    specgram::create_filterbanks(_cfg.num_filters, _cfg.frame_length_tn, _cfg.sample_freq_hz,
                                 _filterbank);
    _noisemaker = make_shared<noise_clips>(_cfg.noise_index_file);
}

audio::transformer::~transformer()
{
}

std::shared_ptr<audio::decoded> audio::transformer::transform(
                                      std::shared_ptr<audio::params> params,
                                      std::shared_ptr<audio::decoded> decoded)
{
    cv::Mat& samples_mat = decoded->get_time_data()->get_data();
    _noisemaker->addNoise(samples_mat,
                          params->add_noise,
                          params->noise_index,
                          params->noise_offset_fraction,
                          params->noise_level); // no-op if no noise files

    // convert from time domain to frequency domain into the freq mat
    specgram::wav_to_specgram(samples_mat,
                              _cfg.frame_length_tn,
                              _cfg.frame_stride_tn,
                              _cfg.time_steps,
                              _window,
                              decoded->get_freq_data());
    if (_cfg.feature_type != "specgram") {
        cv::Mat tmpmat;
        specgram::specgram_to_cepsgram(decoded->get_freq_data(), _filterbank, tmpmat);
        if (_cfg.feature_type == "mfcc") {
            specgram::cepsgram_to_mfcc(tmpmat, _cfg.num_cepstra, decoded->get_freq_data());
        } else {
            decoded->get_freq_data() = tmpmat;
        }
    }

    // place into a destination with the appropriate time dimensions
    cv::Mat resized;
    cv::resize(decoded->get_freq_data(), resized, cv::Size(), 1.0, params->time_scale_fraction,
               (params->time_scale_fraction > 1.0) ? CV_INTER_CUBIC : CV_INTER_AREA);
    decoded->get_freq_data() = resized;
    decoded->valid_frames = std::min((uint32_t) resized.rows, (uint32_t) _cfg.time_steps);

    return decoded;
}

void audio::loader::load(const vector<void*>& outbuf, shared_ptr<audio::decoded> input)
{
    auto nframes = input->valid_frames;
    auto frames = input->get_freq_data();
    int cv_type = _cfg.get_shape_type().get_otype().cv_type;
    cv::Mat padded_frames(_cfg.time_steps, _cfg.freq_steps, frames.type());

    frames(cv::Range(0, nframes), cv::Range::all()).copyTo(
        padded_frames(cv::Range(0, nframes), cv::Range::all()));

    if (nframes < _cfg.time_steps) {
        padded_frames(cv::Range(nframes, _cfg.time_steps), cv::Range::all()) = cv::Scalar::all(0);
    }

    cv::normalize(padded_frames, padded_frames, 0, 255, CV_MINMAX, CV_8UC1);
    cv::Mat tmp(padded_frames.size(), cv_type);
    padded_frames.copyTo(tmp);

    cv::Mat dst(_cfg.freq_steps, _cfg.time_steps, cv_type, (void *) outbuf[0]);
    cv::transpose(tmp, dst);
    cv::flip(dst, dst, 0);
}

