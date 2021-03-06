# ******************************************************************************
# Copyright 2017-2018 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ******************************************************************************

set(AEON_SOURCE_FILES
    async_manager.cpp
    augment_audio.cpp
    augment_image.cpp
    avi.cpp
    base64.cpp
    batch_decoder.cpp
    batch_iterator.cpp
    block.cpp
    block_loader_file.cpp
    block_loader_nds.cpp
    block_manager.cpp
    box.cpp
    boundingbox.cpp
    buffer_batch.cpp
    cache_system.cpp
    cap_mjpeg_decoder.cpp
    cpio.cpp
    crc.cpp
    etl_audio.cpp
    etl_boundingbox.cpp
    etl_char_map.cpp
    etl_depthmap.cpp
    etl_image.cpp
    etl_label_map.cpp
    etl_localization_rcnn.cpp
    etl_localization_ssd.cpp
    etl_pixel_mask.cpp
    etl_video.cpp
    file_util.cpp
    image.cpp
    interface.cpp
    loader.cpp
    log.cpp
    manifest_file.cpp
    manifest_nds.cpp
    noise_clips.cpp
    normalized_box.cpp
    provider.cpp
    provider_factory.cpp
    specgram.cpp
    typemap.cpp
    util.cpp
    wav_data.cpp
    web_app.cpp
    web_server.cpp
    event.cpp
    raw_image.cpp
    output_saver.cpp)

if (ENABLE_AEON_CLIENT)
    list(APPEND AEON_SOURCE_FILES
        client/curl_connector.cpp
        client/loader_remote.cpp
        client/service.cpp
        client/remote_config.cpp)
endif()

if (ENABLE_OPENFABRICS_CONNECTOR)
    list(APPEND AEON_SOURCE_FILES
         rdma/ofi.cpp
         client/ofi_connector.cpp)
endif()

set(AEON_HEADER_FILE_NAME aeon.hpp)
set(AEON_HEADER_FILE_IN ${AEON_HEADER_FILE_NAME}.in)
set(AEON_HEADER_FILE ${CMAKE_CURRENT_BINARY_DIR}/${AEON_HEADER_FILE_NAME})

configure_file(${AEON_HEADER_FILE_IN} ${AEON_HEADER_FILE} @ONLY)

set(AEON_INSTALLABLE_HEADER_FILES
    ${AEON_HEADER_FILE})

set(AEON_HEADER_FILES
    ${AEON_INSTALLABLE_HEADER_FILES})

set(AEON_PKGCONFIG_FILE_NAME libaeon.pc)
set(AEON_PKGCONFIG_FILE ${CMAKE_CURRENT_BINARY_DIR}/${AEON_PKGCONFIG_FILE_NAME})
set(AEON_PKGCONFIG_FILE_IN ${AEON_PKGCONFIG_FILE_NAME}.in)

configure_file(${AEON_PKGCONFIG_FILE_IN} ${AEON_PKGCONFIG_FILE} @ONLY)

set(AEON_CMAKE_FIND_PACKAGE
    ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Modules/AeonConfig.cmake)

if (PYTHON_FOUND)
    list(APPEND AEON_SOURCE_FILES
        api.cpp)
endif()

if (PYTHON_PLUGIN)
    list(APPEND AEON_SOURCE_FILES
        python_plugin.cpp
        conversion.cpp
        python_utils.cpp)
endif()

set(SETUP_PY_IN "${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in")
set(SETUP_PY    "${CMAKE_CURRENT_BINARY_DIR}/../setup.py")
configure_file(${SETUP_PY_IN} ${SETUP_PY})

add_library(aeon SHARED ${AEON_SOURCE_FILES} ${AEON_HEADER_FILES})

install(TARGETS aeon DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${AEON_INSTALLABLE_HEADER_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/aeon/)
install(FILES ${AEON_PKGCONFIG_FILE} DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig/)
install(FILES ${AEON_CMAKE_FIND_PACKAGE} DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Aeon/)

set(AEON_LIBRARIES ${Boost_LIBRARIES} ${SOX_LIBRARIES} ${CURL_LIBRARIES} ${OpenCV_LIBRARIES} ${PYTHON_LIBRARIES})
if (ENABLE_OPENFABRICS_CONNECTOR)
    list(APPEND AEON_LIBRARIES ${OPENFABRICS_LIBRARIES})
endif()

target_link_libraries(aeon ${AEON_LIBRARIES})
set_target_properties(aeon PROPERTIES VERSION ${AEON_VERSION_MAJOR}.${AEON_VERSION_MINOR}.${AEON_VERSION_PATCH}
                                      SOVERSION ${AEON_VERSION_MAJOR})

