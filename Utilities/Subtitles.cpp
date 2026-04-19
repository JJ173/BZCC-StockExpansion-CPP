#include "Subtitles.h"

#include <cstring>

#include "../source/fun3d/ScriptUtils.h"

const char* Subtitles::GetSubtitlePanelName(const GameConfig::SubtitlePanelSize panel_size)
{
    switch (panel_size)
    {
    case GameConfig::SubtitlePanelSize::Large:
        return "SubtitlesPanel_Large";
    case GameConfig::SubtitlePanelSize::Medium:
        return "SubtitlesPanel_Medium";
    case GameConfig::SubtitlePanelSize::Small:
        return "SubtitlesPanel";
    }

    return "SubtitlesPanel";
}

int Subtitles::AudioWithSubtitles(const char* clip_name, const GameConfig::SubtitlePanelSize panel_size)
{
    audio_clip_ = AudioMessage(clip_name);

    const bool is_subtitles_enabled = GetVarItemInt(GameConfig::OPTIONS_PLAY_SUBTITLES) > 0;
    if (!is_subtitles_enabled)
    {
        return audio_clip_;
    }

    char subtitle_to_use[GameConfig::MAX_NAME_LENGTH];
    CreateSubtitleName(clip_name, subtitle_to_use, sizeof(subtitle_to_use));

    subtitle_panel_size_ = panel_size;
    subtitle_to_use_ = subtitle_to_use;
    start_subtitles_ = true;

    return audio_clip_;
}

void Subtitles::CreateSubtitleName(const char* clip_name, char* out, const size_t out_size)
{
    const size_t len = std::strlen(clip_name);
    int written;

    if (len >= 4 && std::strcmp(clip_name + len - 4, ".wav") == 0)
    {
        written = sprintf_s(out, out_size, "%.*s_subtitle.txt", static_cast<int>(len - 4), clip_name);
    }
    else
    {
        written = sprintf_s(out, out_size, "%s_subtitle.txt", clip_name);
    }

    if (written < 0)
    {
        out[0] = '\0';
    }
}

void Subtitles::Execute()
{
    if (start_subtitles_)
    {
        if (!subtitles_loaded_)
        {
            IFace_Exec("bzgame_subtitles.cfg");
            subtitles_loaded_ = true;
        }

        active_subtitle_panel_ = GetSubtitlePanelName(subtitle_panel_size_);
        LoadSubtitles(active_subtitle_panel_, subtitle_to_use_.c_str());
        IFace_Activate(active_subtitle_panel_);

        start_subtitles_ = false;
    }

    if (audio_clip_ != 0 && IsAudioMessageDone(audio_clip_))
    {
        if (active_subtitle_panel_ != nullptr)
        {
            IFace_Deactivate(active_subtitle_panel_);
        }

        active_subtitle_panel_ = nullptr;
        audio_clip_ = 0;
        subtitle_panel_size_ = GameConfig::SubtitlePanelSize::Small;
        subtitle_to_use_.clear();
    }
}

void Subtitles::LoadSubtitles(const char* list_box, const char* subtitle_file)
{
    IFace_ClearListBox(list_box);

    char buffer[GameConfig::MAX_SUBTITLE_LENGTH] = {};
    size_t buf_size = sizeof(buffer);

    if (!LoadFile(subtitle_file, buffer, buf_size))
    {
        return;
    }

    if (buf_size >= sizeof(buffer))
    {
        buf_size = sizeof(buffer) - 1;
    }

    buffer[buf_size] = '\0';

    const char* line_start = buffer;
    for (char* p = buffer; ; ++p)
    {
        if (*p == '\n' || *p == '\0')
        {
            const char saved = *p;
            *p = '\0';

            char* line_end = p;
            if (line_end > line_start && *(line_end - 1) == '\r')
            {
                *(line_end - 1) = '\0';
            }

            if (*line_start != '\0')
            {
                IFace_AddTextItem(list_box, line_start);
            }

            if (saved == '\0')
            {
                break;
            }

            line_start = p + 1;
        }
    }
}
