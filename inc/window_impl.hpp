//
// Created by romaonishuk on 01.12.19.
//

#ifndef LEXI_WINDOW_IMPL_HPP
#define LEXI_WINDOW_IMPL_HPP

#include <set>
#include <string>

#include "types.hpp"

namespace Lexi {
struct Font;
struct FontName;
}  // namespace Lexi

namespace Gui {
class WindowImpl
{
public:
    WindowImpl() = default;
    virtual ~WindowImpl() = default;

    virtual void DrawRectangle(const Point& point, const width_t width, const height_t height) = 0;
    virtual void DrawText(const Point& text_position, std::string text) = 0;
    virtual void DrawLine(const Point& start_point, const Point& end_point) = 0;
    virtual void FillRectangle(const Point& point, const width_t width, const height_t height, const Color color) = 0;

    virtual void SetForeground(const int color) = 0;

    virtual void ShowWindow() = 0;
    virtual void HideWindow() = 0;
    virtual void ClearWindow() = 0;
    virtual void ClearGlyph(const GlyphParams& p, bool sendExposureEvent) = 0;
    virtual void Resize(width_t width, height_t height) = 0;

    virtual std::set<Lexi::FontName> GetFontList() = 0;
    virtual bool ChangeFont(Lexi::Font&) = 0;
    virtual void SetFontPath(const std::string& path) = 0;

    // TODO(rmn): Wrap
    virtual unsigned long GetWindow() const = 0;
    virtual void* GetDisplay() const = 0;

    virtual void DrawText(const GlyphParams& params, const std::string& text, Alignment alignment) = 0;
};
}  // namespace Gui
#endif  // LEXI_WINDOW_IMPL_HPP
