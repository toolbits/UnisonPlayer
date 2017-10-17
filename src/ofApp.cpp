/*
**      UnisonPlayer
**
**      Original Copyright (C) 2017 - 2017 HORIGUCHI Junshi.
**                                          http://iridium.jp/
**                                          zap00365@nifty.com
**      Portions Copyright (C) <year> <author>
**                                          <website>
**                                          <e-mail>
**      Version     openFrameworks
**      Website     http://iridium.jp/
**      E-mail      zap00365@nifty.com
**
**      This source code is for Xcode.
**      Xcode 9.0 (Apple LLVM 9.0.0)
**
**      ofApp.cpp
**
**      ------------------------------------------------------------------------
**
**      The MIT License (MIT)
**
**      Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
**      associated documentation files (the "Software"), to deal in the Software without restriction,
**      including without limitation the rights to use, copy, modify, merge, publish, distribute,
**      sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
**      furnished to do so, subject to the following conditions:
**      The above copyright notice and this permission notice shall be included in all copies or
**      substantial portions of the Software.
**      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
**      BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
**      IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
**      WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
**      OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**
**      以下に定める条件に従い、本ソフトウェアおよび関連文書のファイル（以下「ソフトウェア」）の複製を
**      取得するすべての人に対し、ソフトウェアを無制限に扱うことを無償で許可します。
**      これには、ソフトウェアの複製を使用、複写、変更、結合、掲載、頒布、サブライセンス、および、または販売する権利、
**      およびソフトウェアを提供する相手に同じことを許可する権利も無制限に含まれます。
**      上記の著作権表示および本許諾表示を、ソフトウェアのすべての複製または重要な部分に記載するものとします。
**      ソフトウェアは「現状のまま」で、明示であるか暗黙であるかを問わず、何らの保証もなく提供されます。
**      ここでいう保証とは、商品性、特定の目的への適合性、および権利非侵害についての保証も含みますが、それに限定されるものではありません。
**      作者または著作権者は、契約行為、不法行為、またはそれ以外であろうと、ソフトウェアに起因または関連し、
**      あるいはソフトウェアの使用またはその他の扱いによって生じる一切の請求、損害、その他の義務について何らの責任も負わないものとします。
*/

#include "ofApp.h"
#include "ofxXmlSettings.h"

#define COLOR_BACKGROUND    (ofColor(0, 0, 0))
#define COLOR_PREVIEW       (ofColor(63, 63, 63))
#define COLOR_EMPTY         (ofColor(255, 0, 255))
#define COLOR_PLAY          (ofColor(0, 255, 0))
#define COLOR_WAIT          (ofColor(255, 255, 0))
#define COLOR_STOP          (ofColor(255, 0, 0))
#define MARGIN_HORIZONTAL   (10)
#define MARGIN_VERTICAL     (108)

void ofApp::setup(void)
{
    ofxXmlSettings config;
    vector<ofRectangle>::const_iterator mit;
    vector<ofVideoPlayer>::const_iterator vit;
    vector<ofRectangle>::const_iterator rit;
    string file;
    ofRectangle rect;
    int limit;
    int count;
    int i;
    bool check;
    
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofEnableSmoothing();
    ofEnableAntiAliasing();
    ofEnableAlphaBlending();
    ofSetBackgroundAuto(false);
    ofBackground(COLOR_BACKGROUND);
    ofSetDataPathRoot(ofFilePath::join(ofFilePath::join(ofFilePath::getUserHomeDir(), "Documents"), ofFilePath::getBaseName(ofFilePath::getCurrentExePath())));
    if (config.load(CONFIG_FILE)) {
        check = false;
        if (config.pushTag("display")) {
            ofSetFrameRate(config.getValue("framerate", 60));
            ofSetVerticalSync(!!config.getValue("vsync", true));
            check = true;
            config.popTag();
        }
        if (check) {
            check = false;
            if (config.pushTag("config")) {
                _preview = !!config.getValue("preview", true);
                _autorun = !!config.getValue("autorun", false);
                _loop = !!config.getValue("loop", true);
                check = true;
                config.popTag();
            }
            if (check) {
                _monitor = getMonitorInfo(&limit, NULL);
                if (limit > 0) {
                    if (_preview) {
                        --limit;
                    }
                    if (config.pushTag("resource")) {
                        count = config.getNumTags("video");
                        if (limit > 0) {
                            if (count > limit) {
                                count = limit;
                            }
                        }
                        for (i = 0; i < count; ++i) {
                            if (config.pushTag("video", i)) {
                                file = config.getValue("file", "");
                                _video.push_back(ofVideoPlayer());
                                if (_video.back().load(file)) {
                                    _video.back().setLoopState(OF_LOOP_NONE);
                                    _video.back().setVolume(config.getValue("volume", 1.0));
                                    _video.back().firstFrame();
                                }
                                else {
                                    _video.pop_back();
                                    ofSystemAlertDialog(string("動画ファイルの読み込みに失敗しました.\n\n") + file);
                                }
                                config.popTag();
                            }
                        }
                        config.popTag();
                    }
                    _whole.set(0, 0, 0, 0);
                    if (limit > 0) {
                        mit = _monitor.begin();
                        if (_preview) {
                            ++mit;
                        }
                        for (; mit != _monitor.end(); ++mit) {
                            _rect.push_back(*mit);
                            _whole = (_whole.isEmpty()) ? (*mit) : (_whole.getUnion(*mit));
                        }
                    }
                    else {
                        for (vit = _video.begin(); vit != _video.end(); ++vit) {
                            rect.set(_whole.width, 0, vit->getWidth(), vit->getHeight());
                            _rect.push_back(rect);
                            _whole = (_whole.isEmpty()) ? (rect) : (_whole.getUnion(rect));
                        }
                    }
                    for (rit = _rect.begin(); rit != _rect.end(); ++rit) {
                        rit->x -= _whole.x;
                        rit->y -= _whole.y;
                    }
                    _state = (_autorun) ? (STATE_LOAD) : (STATE_STOP);
                    _debug = !!config.getValue("debug", true);
                    ofHideCursor();
                }
                else {
                    ofExit(4);
                }
            }
            else {
                ofSystemAlertDialog(string("アプリケーション設定の初期化に失敗しました.\nアプリケーションを終了します.\n\n\"") + CONFIG_FILE + "\" ファイルのバージョンが古い場合や\n壊れている場合は, 削除したのち再起動してください.");
                ofExit(3);
            }
        }
        else {
            ofSystemAlertDialog(string("アプリケーション設定の初期化に失敗しました.\nアプリケーションを終了します.\n\n\"") + CONFIG_FILE + "\" ファイルのバージョンが古い場合や\n壊れている場合は, 削除したのち再起動してください.");
            ofExit(2);
        }
    }
    else {
        ofSystemAlertDialog(string("アプリケーション設定の初期化に失敗しました.\nアプリケーションを終了します.\n\n\"") + CONFIG_FILE + "\" ファイルが見つかりません.");
        ofExit(1);
    }
    return;
}

void ofApp::exit(void)
{
    ofShowCursor();
    return;
}

void ofApp::update(void)
{
    vector<ofVideoPlayer>::iterator it;
    
    switch (_state) {
        case STATE_LOAD:
            for (it = _video.begin(); it != _video.end(); ++it) {
                if (!it->isLoaded()) {
                    break;
                }
            }
            if (it == _video.end()) {
                for (it = _video.begin(); it != _video.end(); ++it) {
                    it->play();
                }
                _state = STATE_PLAY;
            }
            break;
        case STATE_PLAY:
            for (it = _video.begin(); it != _video.end(); ++it) {
                if (!it->getIsMovieDone()) {
                    break;
                }
            }
            if (it == _video.end()) {
                _state = (_loop) ? (STATE_LOAD) : (STATE_STOP);
            }
            break;
        default:
            break;
    }
    for (it = _video.begin(); it != _video.end(); ++it) {
        it->update();
    }
    return;
}

void ofApp::draw(void)
{
    vector<ofRectangle>::const_iterator mit;
    vector<ofVideoPlayer>::const_iterator vit;
    vector<ofRectangle>::const_iterator rit;
    string state;
    ofRectangle rect;
    ofPoint offset;
    float scale;
    float duration;
    float position;
    
    ofFill();
    mit = _monitor.begin();
    if (_preview) {
        rect = *mit;
        ofSetColor(COLOR_PREVIEW);
        ofDrawRectangle(rect);
        if (!_whole.isEmpty()) {
            deflateRect(&rect, MARGIN_HORIZONTAL, MARGIN_VERTICAL);
            fitCoordinate(rect, _whole, &offset, &scale);
            ofPushMatrix();
            ofTranslate(offset);
            ofScale(scale, scale);
            vit = _video.begin();
            for (rit = _rect.begin(); rit != _rect.end(); ++rit) {
                if (vit != _video.end()) {
                    ofSetColor(255, 255, 255);
                    vit->draw(*rit);
                    ofSetColor(0, 0, 0, 127);
                    ofDrawRectangle(rit->x, rit->y, rit->width, 33 / scale);
                    ofDrawRectangle(rit->x, rit->y + rit->height - 19 / scale, rit->width, 19 / scale);
                    ofSetColor(255, 255, 255);
                    ofDrawBitmapString(vit->getMoviePath(), rit->x + 5 / scale, rit->y + 14 / scale);
                    ofDrawBitmapString("[movie] " + ofToString(vit->getWidth()) + " * " + ofToString(vit->getHeight()) + " [monitor] " + ofToString(rit->width) + " * " + ofToString(rit->height), rit->x + 3 / scale, rit->y + 28 / scale);
                    switch (_state) {
                        case STATE_STOP:
                            ofSetColor(COLOR_STOP);
                            state = "[ STOP ]";
                            break;
                        default:
                            if (vit->getIsMovieDone()) {
                                ofSetColor(COLOR_WAIT);
                                state = "[ WAIT ]";
                            }
                            else {
                                ofSetColor(COLOR_PLAY);
                                state = (ofGetElapsedTimeMillis() / 500 % 2 == 0) ? ("[ PLAY ]") : ("[      ]");
                            }
                            break;
                    }
                    ofDrawBitmapString(state, rit->x + 3 / scale, rit->y + rit->height - 5 / scale);
                    ofSetColor(255, 255, 255);
                    duration = vit->getDuration();
                    position = vit->getPosition();
                    if (isnan(position) || isinf(position)) {
                        position = 1.0;
                    }
                    ofDrawBitmapString(ofToString(position * duration, 2) + " / " + ofToString(duration, 2) + " sec" + ((_loop) ? (" [loop]") : ("")), rit->x + 78 / scale, rit->y + rit->height - 5 / scale);
                    ++vit;
                }
                else {
                    ofSetColor(COLOR_EMPTY);
                    ofDrawRectangle(*rit);
                }
            }
            ofPopMatrix();
        }
        ++mit;
    }
    vit = _video.begin();
    for (; mit != _monitor.end(); ++mit) {
        if (vit != _video.end()) {
            ofSetColor(255, 255, 255);
            vit->draw(*mit);
            ++vit;
        }
        else {
            ofSetColor(COLOR_BACKGROUND);
            ofDrawRectangle(*mit);
        }
    }
    if (_debug) {
        mit = _monitor.begin();
        ofSetColor(255, 255, 255);
        ofDrawBitmapString(ofToString(ofGetFrameRate(), 2, 6, '0') + " FPS", mit->x + 10, mit->y + 18);
        ofDrawBitmapString("space: start playing", mit->x + 10, mit->y + mit->height - 80);
        ofDrawBitmapString("    L: enable / disable auto loop", mit->x + 10, mit->y + mit->height - 66);
        ofDrawBitmapString("    D: show / hide debug info", mit->x + 10, mit->y + mit->height - 52);
        ofDrawBitmapString("  esc: quit application", mit->x + 10, mit->y + mit->height - 38);
        ofDrawBitmapString("UnisonPlayer 1.0.3   Copyright (C) 2017 HORIGUCHI Junshi. All rights reserved.   http://iridium.jp", mit->x + 10, mit->y + mit->height - 10);
    }
    return;
}

void ofApp::keyPressed(int key)
{
    switch (key) {
        case ' ':
            if (_state == STATE_STOP) {
                _state = STATE_LOAD;
            }
            break;
        case 'L':
        case 'l':
            _loop = !_loop;
            break;
        case 'D':
        case 'd':
            _debug = !_debug;
            break;
    }
    return;
}

vector<ofRectangle> ofApp::getMonitorInfo(int* count, ofRectangle* whole)
{
    GLFWmonitor** monitor;
    GLFWvidmode const* mode;
    ofRectangle rect;
    vector<ofRectangle> temp;
    int c;
    int x;
    int y;
    int i;
    vector<ofRectangle> result;
    
    monitor = glfwGetMonitors(&c);
    if (c > 0) {
        rect.set(FLT_MAX, FLT_MAX, FLT_MIN, FLT_MIN);
        for (i = 0; i < c; ++i) {
            mode = glfwGetVideoMode(monitor[i]);
            glfwGetMonitorPos(monitor[i], &x, &y);
            if (x < rect.x) {
                rect.x = x;
            }
            if (y < rect.y) {
                rect.y = y;
            }
            if (x + mode->width > rect.width) {
                rect.width = x + mode->width;
            }
            if (y + mode->height > rect.height) {
                rect.height = y + mode->height;
            }
            temp.push_back(ofRectangle(x, y, mode->width, mode->height));
        }
        rect.width -= rect.x;
        rect.height -= rect.y;
        for (i = 0; i < temp.size(); ++i) {
            temp[i].x -= rect.x;
            temp[i].y -= rect.y;
        }
        rect.x = 0;
        rect.y = 0;
        if (count != NULL) {
            *count = c;
        }
        if (whole != NULL) {
            *whole = rect;
        }
        while (temp.size() > 0) {
            rect.x = FLT_MAX;
            c = -1;
            for (i = 0; i < temp.size(); ++i) {
                if (temp[i].x < rect.x) {
                    rect.x = temp[i].x;
                    c = i;
                }
            }
            if (c > -1) {
                result.push_back(temp[c]);
                temp.erase(temp.begin() + c);
            }
        }
    }
    else {
        if (count != NULL) {
            *count = 0;
        }
        if (whole != NULL) {
            whole->set(0, 0, 0, 0);
        }
    }
    return result;
}

void ofApp::deflateRect(ofRectangle* rect, float l, float u, float r, float b)
{
    rect->x += l;
    rect->y += u;
    rect->width -= l + r;
    rect->height -= u + b;
    return;
}

void ofApp::fitCoordinate(ofRectangle const& outer, ofRectangle const& inner, ofPoint* offset, float* scale)
{
    ofRectangle rect;
    
    rect = inner;
    rect.scaleTo(outer);
    *offset = rect.getPosition();
    *scale = (outer.getAspectRatio() <= rect.getAspectRatio()) ? (outer.width / inner.width) : (outer.height / inner.height);
    return;
}
