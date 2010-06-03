/***************************************************************************
                          enginechannel.cpp  -  description
                             -------------------
    begin                : Sun Apr 28 2002
    copyright            : (C) 2002 by
    email                :
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "controlpushbutton.h"
#include "enginebuffer.h"
#include "enginevinylsoundemu.h"
#include "enginechannel.h"
#include "engineclipping.h"
#include "enginepregain.h"
#include "enginevolume.h"
#include "engineflanger.h"
#include "enginefilterblock.h"
#include "enginevumeter.h"
#include "enginefilteriir.h"

EngineChannel::EngineChannel(const char* group,
                             ConfigObject<ConfigValue>* pConfig,
                             EngineChannel::ChannelOrientation defaultOrientation)
        : m_pConfig(pConfig) {
    m_pPregain = new EnginePregain(group);
    m_pFilter = new EngineFilterBlock(group);
    m_pFlanger = new EngineFlanger(group);
    m_pClipping = new EngineClipping(group);
    m_pBuffer = new EngineBuffer(group, pConfig);
    m_pVinylSoundEmu = new EngineVinylSoundEmu(pConfig, group);
    m_pVolume = new EngineVolume(ConfigKey(group, "volume"));
    m_pVUMeter = new EngineVuMeter(group);
    m_pPFL = new ControlPushButton(ConfigKey(group, "pfl"), true);
    m_pOrientation = new ControlObject(ConfigKey(group, "orientation"));
    m_pOrientation->set(defaultOrientation);

    // Create the 'transform' control. It isn't used anymore, but is needed for
    // backwards compatibility.
    new ControlPushButton(ConfigKey(group, "transform"));
}

EngineChannel::~EngineChannel() {
    delete m_pBuffer;
    delete m_pClipping;
    delete m_pFilter;
    delete m_pFlanger;
    delete m_pPregain;
    delete m_pVinylSoundEmu;
    delete m_pVolume;
    delete m_pVUMeter;
    delete m_pPFL;
}

bool EngineChannel::isPFL() {
    return m_pPFL->get() == 1.0;
}

void EngineChannel::process(const CSAMPLE*, const CSAMPLE * pOut, const int iBufferSize) {
    // Process the raw audio
    m_pBuffer->process(0, pOut, iBufferSize);
    // Emulate vinyl sounds
    m_pVinylSoundEmu->process(pOut, pOut, iBufferSize);
    // Apply pregain
    m_pPregain->process(pOut, pOut, iBufferSize);
    // Filter the channel with EQs
    m_pFilter->process(pOut, pOut, iBufferSize);
    // TODO(XXX) LADSPA
    m_pFlanger->process(pOut, pOut, iBufferSize);
    // Apply clipping
    m_pClipping->process(pOut, pOut, iBufferSize);
    // Update VU meter
    m_pVUMeter->process(pOut, pOut, iBufferSize);
    // Apply channel volume
    m_pVolume->process(pOut, pOut, iBufferSize);
}

EngineBuffer* EngineChannel::getEngineBuffer() {
    return m_pBuffer;
}

EngineChannel::ChannelOrientation EngineChannel::getOrientation() {
    double dOrientation = m_pOrientation->get();
    if (dOrientation == LEFT) {
        return LEFT;
    } else if (dOrientation == CENTER) {
        return CENTER;
    } else if (dOrientation == RIGHT) {
        return RIGHT;
    }
    return CENTER;
}

void EngineChannel::setPitchIndpTimeStretch(bool b) {
    m_pBuffer->setPitchIndpTimeStretch(b);
}
