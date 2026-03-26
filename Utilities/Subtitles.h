#pragma once
#include <string>

#include "../Shared/GameConfig.h"

class Subtitles
{
    bool subtitles_loaded_ = false;
    bool start_subtitles_ = false;
    
    std::string subtitle_to_use_;
    
    int audio_clip_ = 0;
    
    GameConfig::SubtitlePanelSize subtitle_panel_size_ = GameConfig::SubtitlePanelSize::Small;
    const char* active_subtitle_panel_ = nullptr;

    static void CreateSubtitleName(const char* clip_name, char* out, size_t out_size);
    static void LoadSubtitles(const char* list_box, const char* subtitle_file);
    static const char* GetSubtitlePanelName(GameConfig::SubtitlePanelSize panel_size);
    
public:
    int AudioWithSubtitles(const char* clip_name, GameConfig::SubtitlePanelSize panel_size);
    void Execute();
};
