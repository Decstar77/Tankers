#include "game_ui.h"

// static UIState uiState = {};

// void UIStateReset() {
//     uiState.surfaceMouse = { GetMousePosition().x, GetMousePosition().y };
//     uiState.elementHovered = false;
//     uiState.submissions.Clear();
// }

// void UIStateDraw() {
//     const i32 count = uiState.submissions.GetCount();
//     for (i32 i = 0; i < count; i++) {
//         UISubmission & submission = uiState.submissions[i];
//         switch (submission.type) {
//         case UI_SUBMISSION_TYPE_BUTTON: {
//             if (submission.isHovered) {
//                 if (submission.isPressed) {
//                     DrawRectangleRec(submission.rect, submission.pressedColor);
//                 }
//                 else {
//                     DrawRectangleRec(submission.rect, submission.hoveredColor);
//                 }
//             }
//             else {
//                 DrawRectangleRec(submission.rect, submission.baseColor);
//             }

//             Vector2 textPos = { submission.rect.x + submission.rect.width / 2 - submission.textSize.x / 2,
//             submission.rect.y + submission.rect.height / 2 - submission.textSize.y / 2 };
//             DrawTextEx(GetFontDefault(), submission.text.GetCStr(), textPos, 20, 1, BLACK);

//         } break;
//         case UI_SUBMISSION_TYPE_BLOCK_BUTTON: {
//             if (submission.isHovered) {
//                 if (submission.isPressed) {
//                     DrawRectangleRec(submission.rect, submission.pressedColor);
//                 }
//                 else {
//                     DrawRectangleRec(submission.rect, submission.hoveredColor);
//                 }
//             }
//             else {
//                 DrawRectangleRec(submission.rect, submission.baseColor);
//             }

//             Vector2 textPos = { submission.rect.x + submission.rect.width / 2 - submission.textSize.x / 2,
//             submission.rect.y + submission.rect.height / 2 - submission.textSize.y / 2 };
//             DrawTextEx(GetFontDefault(), submission.text.GetCStr(), textPos, 20, 1, BLACK);
//         } break;
//         default: {
//             Assert(0 && "Invalid submission type");
//         }
//         }
//     }
// }

// void UIColorsCreate() {
//     uiState.uiColors.counts[UI_COLOR_SLOT_BACKGROUND] = 1;
//     uiState.uiColors.stacks[UI_COLOR_SLOT_BACKGROUND][0] = LIGHTGRAY;
// }

// void UIColorsPush(UIColorSlot slot, Color c) {
//     Assert(uiState.uiColors.counts[slot] < ArrayCount(uiState.uiColors.stacks[slot]));
//     Assert(slot != UI_COLOR_SLOT_INVALID);
//     uiState.uiColors.stacks[slot][uiState.uiColors.counts[slot]] = c;
//     uiState.uiColors.counts[slot]++;
// }

// void UIColorsPop(UIColorSlot slot) {
//     Assert(uiState.uiColors.counts[slot] > 0);
//     Assert(slot != UI_COLOR_SLOT_INVALID);
//     uiState.uiColors.counts[slot]--;
// }

// Color UIColorsGet(UIColorSlot slot) {
//     Assert(uiState.uiColors.counts[slot] > 0);
//     Assert(slot != UI_COLOR_SLOT_INVALID);
//     return uiState.uiColors.stacks[slot][uiState.uiColors.counts[slot] - 1];
// }

// bool UIDrawButtonCenter(i32 centerX, i32 centerY, const char * text, Rectangle * bb) {
//     Vector2 textSize = MeasureTextEx(GetFontDefault(), text, 20, 1);
//     Vector2 rectSize = { textSize.x + 20, textSize.y + 20 };
//     Rectangle rect = { (float)centerX - rectSize.x / 2, (float)centerY - rectSize.y / 2, rectSize.x, rectSize.y };

//     if (bb != nullptr) {
//         *bb = rect;
//     }

//     Vector2 mousePos = { uiState.surfaceMouse.x, uiState.surfaceMouse.y };

//     UISubmission submission = {};
//     submission.type = UI_SUBMISSION_TYPE_BUTTON;
//     submission.text = text;
//     submission.textSize = textSize;
//     submission.rect = rect;
//     submission.baseColor = UIColorsGet(UI_COLOR_SLOT_BACKGROUND);
//     submission.hoveredColor = submission.baseColor;
//     submission.hoveredColor.r = (u8)((f32)submission.hoveredColor.r * 1.2f);
//     submission.hoveredColor.g = (u8)((f32)submission.hoveredColor.g * 1.2f);
//     submission.hoveredColor.b = (u8)((f32)submission.hoveredColor.b * 1.2f);
//     submission.pressedColor = submission.baseColor;
//     submission.pressedColor.r = (u8)((f32)submission.pressedColor.r * 1.5f);
//     submission.pressedColor.g = (u8)((f32)submission.pressedColor.g * 1.5f);
//     submission.pressedColor.b = (u8)((f32)submission.pressedColor.b * 1.5f);
//     submission.isHovered = CheckCollisionPointRec(mousePos, rect);
//     submission.isPressed = submission.isHovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

//     uiState.submissions.Add(submission);
//     uiState.elementHovered = uiState.elementHovered || submission.isHovered;

//     return submission.isPressed;
// }

// bool UIDrawButtonTopLeft(i32 topLeft, i32 topRight, const char * text, Rectangle * bb) {
//     Vector2 textSize = MeasureTextEx(GetFontDefault(), text, 20, 1);
//     Vector2 rectSize = { textSize.x + 20, textSize.y + 20 };
//     i32 cx = (i32)(topLeft + rectSize.x / 2);
//     i32 cy = (i32)(topRight + rectSize.y / 2);
//     return UIDrawButtonCenter(cx, cy, text, bb);
// }

// bool UIDrawBlockButton(v2 center, v2 dims, Color c, const char * text) {
//     UISubmission submission = {};
//     submission.type = UI_SUBMISSION_TYPE_BLOCK_BUTTON;
//     submission.text = text;
//     submission.textSize = MeasureTextEx(GetFontDefault(), text, 20, 1);
//     submission.rect = { center.x - dims.x / 2, center.y - dims.y / 2, dims.x, dims.y };
//     submission.baseColor = c;
//     submission.hoveredColor = c;
//     submission.hoveredColor.r = (u8)((f32)submission.hoveredColor.r * 1.2f);
//     submission.hoveredColor.g = (u8)((f32)submission.hoveredColor.g * 1.2f);
//     submission.hoveredColor.b = (u8)((f32)submission.hoveredColor.b * 1.2f);
//     submission.pressedColor = c;
//     submission.pressedColor.r = (u8)((f32)submission.pressedColor.r * 1.5f);
//     submission.pressedColor.g = (u8)((f32)submission.pressedColor.g * 1.5f);
//     submission.pressedColor.b = (u8)((f32)submission.pressedColor.b * 1.5f);
//     submission.isHovered = CheckCollisionPointRec({ uiState.surfaceMouse.x, uiState.surfaceMouse.y }, submission.rect);
//     submission.isPressed = submission.isHovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

//     uiState.submissions.Add(submission);
//     uiState.elementHovered = uiState.elementHovered || submission.isHovered;

//     return submission.isPressed;
// }

// bool UIElementHover() {
//     return uiState.elementHovered;
// }
