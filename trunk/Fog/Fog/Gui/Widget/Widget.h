// [Fog-Gui]
//
// [License]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GUI_WIDGET_WIDGET_H
#define _FOG_GUI_WIDGET_WIDGET_H

// [Dependencies]
#include <Fog/Core/Collection/Hash.h>
#include <Fog/Core/Collection/List.h>
#include <Fog/Core/DateTime/Time.h>
#include <Fog/Core/Threading/Lock.h>
#include <Fog/Core/Threading/Thread.h>
#include <Fog/Core/System/Event.h>
#include <Fog/Core/System/Object.h>
#include <Fog/G2d/Font/Font.h>
#include <Fog/G2d/Geometry/Point.h>
#include <Fog/G2d/Geometry/Rect.h>
#include <Fog/G2d/Painting/Painter.h>
#include <Fog/Gui/Global/Constants.h>
#include <Fog/Gui/Layout/LayoutItem.h>
#include <Fog/Gui/Layout/LayoutHint.h>
#include <Fog/Gui/Widget/Event.h>

namespace Fog {

//! @addtogroup Fog_Gui_Widget
//! @{

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct Layout;
struct GuiEngine;
struct GuiWindow;
struct Widget;
struct Window;

// ============================================================================
// [Fog::Widget]
// ============================================================================

//! @brief Base class for creating visual components that can be painted on the
//! screen and that can catch events generated by windowing system.
//!
//! @c Fog::Widget is based on @c Fog::LayoutItem and layout item is based
//! on @c Fog::Object. Layout item allows to very easy set or calculate layout
//! metrics for each widget. And @c Fog::Object allows to use event mechanism
//! and object introspection that's implemented in Core library.
//!
//! @c Fog::Widget event base is a lot increased compared to @c Fog::Object. It
//! contains these new event handlers:
//!  - <code>virtual void onStateChange(StateEvent* e)</code>
//!  - <code>virtual void onVisibilityChange(VisibilityEvent* e)</code>
//!  - <code>virtual void onGeometry(GeometryEvent* e)</code>
//!  - <code>virtual void onFocus(FocusEvent* e)</code>
//!  - <code>virtual void onKey(KeyEvent* e)</code>
//!  - <code>virtual void onMouse(MouseEvent* e)</code>
//!  - <code>virtual void onPaint(PaintEvent* e)</code>
//!  - <code>virtual void onClose(CloseEvent* e)</code>
//!
//! Widget state:
//!
//! Widget visibility is controled by @c StateEvent and can be changed
//! via @c setState(), @c enable() and @c disable() methods. To check
//! if widget is enabled use @c state() method.
//!
//! Widget visibility:
//!
//! Widget visibility is controled by @c VisibilityEvent and can be changed
//! via @c setVisibility(), @c show() and @c hide() methods. To check if widget
//! is visible use @c visibility() method.
struct FOG_API Widget : public LayoutItem
{
  FOG_DECLARE_OBJECT(Widget, LayoutItem)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  Widget(uint32_t createFlags = 0);
  virtual ~Widget();

  // --------------------------------------------------------------------------
  // [Object Hierarchy]
  // --------------------------------------------------------------------------

  virtual err_t _addChild(sysuint_t index, Object* child);
  virtual err_t _removeChild(sysuint_t index, Object* child);

  // --------------------------------------------------------------------------
  // [Widget Hierarchy]
  // --------------------------------------------------------------------------

  //! @brief Get whether the widget has widget parent.
  //!
  //! @sa getParentWidget().
  FOG_INLINE bool hasParentWidget() { return _parentWidget != NULL; }

  //! @brief Get widget parent.
  //!
  //! Widget parent can be equal to object parent, but in some cases it is
  //! different or @c NULL. If widget was added into object that not inherits
  //! from @ref Widget then widget-parent will be set to @c NULL.
  //!
  //! If widget is menu or menu item then widget parent can be @c NULL or
  //! set to popup widget instance when displayed as popup.
  //!
  //! Unlike object hierarchy, widget parent is not related to memory
  //! management. Object is always owned by its object-parent, widget-parent
  //! is just another layer used by Fog-Gui, because any instance that inherits
  //! from @ref Object can be added to widget, for example @ref Timer.
  //!
  FOG_INLINE Widget* getParentWidget() { return _parentWidget; }

  // --------------------------------------------------------------------------
  // [Gui-Window]
  // --------------------------------------------------------------------------

  //! @brief Get whether the widget has @ref GuiWindow.
  FOG_INLINE bool hasGuiWindow() const { return _guiWindow != NULL; }

  //! @brief Get the widget @ref GuiWindow (only for top-level widgets or menus).
  FOG_INLINE GuiWindow* getGuiWindow() const { return _guiWindow; }

  //! @brief Get closest @ref GuiWindow instance.
  //!
  //! Get @ref GuiWindow associated with this widget. If this widget hasn't
  //! @ref GuiWindow then all parents are traversed until it's found, otherwise
  //! @c NULL is returned.
  GuiWindow* getClosestGuiWindow() const;

  //! @brief Create native gui window.
  err_t createWindow(uint32_t createFlags = 0);
  //! @brief Destroy native gui window.
  err_t destroyWindow();

  //! @brief Get native gui window title.
  String getWindowTitle() const;
  //! @brief Set native gui window title.
  err_t setWindowTitle(const String& title);

  //! @brief Get native gui window icon.
  Image getWindowIcon() const;
  //! @brief Set native gui window icon.
  err_t setWindowIcon(const Image& icon);

  //! @brief Get native gui window resize granularity.
  PointI getWindowGranularity() const;
  //! @brief Set native gui window resize granularity.
  err_t setWindowGranularity(const PointI& pt);

  // --------------------------------------------------------------------------
  // [Widget Geometry]
  // --------------------------------------------------------------------------

  FOG_INLINE bool hasNcArea() const
  {
    return _hasNcArea;
  }

  FOG_INLINE void _updateHasNcArea()
  {
    _hasNcArea = !(_clientGeometry.x == 0 &&
                   _clientGeometry.y == 0 &&
                   _clientGeometry.w == _widgetGeometry.w &&
                   _clientGeometry.h == _widgetGeometry.h);
  }

  //! @brief Get widget geometry.
  FOG_INLINE const RectI& getGeometry() const { return _widgetGeometry; }
  //! @brief Get widget position relative to parent.
  FOG_INLINE const PointI& getPosition() const { return _widgetGeometry.getPosition(); }
  //! @brief Get widget size.
  FOG_INLINE const SizeI& getSize() const { return _widgetGeometry.getSize(); }

  //! @brief Get widget left position, this method is equal to @c left().
  FOG_INLINE int getX() const { return _widgetGeometry.x; }
  //! @brief Get widget top position, this method is equal to @c top().
  FOG_INLINE int getY() const { return _widgetGeometry.y; }
  //! @brief Get widget width.
  FOG_INLINE int getWidth() const { return _widgetGeometry.w; }
  //! @brief Get widget height.
  FOG_INLINE int getHeight() const { return _widgetGeometry.h; }

  //! @brief Set widget position to @a pt.
  //!
  //! @note To set widget position and size together use @c setGeometry().
  void setPosition(const PointI& pos);

  //! @brief Set the widget size to @a sz.
  //!
  //! @note To set widget position and size together use @c setGeometry().
  void setSize(const SizeI& size);

  //! @brief Set widget position and size to @a geometry.
  void setGeometry(const RectI& geometry);

  //! @brief Set widget position to @a pt.
  //!
  //! This method is similar to @c setPosition().
  FOG_INLINE void move(const PointI& pt) { setPosition(pt); }

  //! @brief Set widget size to @a size.
  //!
  //! This method is similar to @c setSize().
  FOG_INLINE void resize(const SizeI& size) { setSize(size); }

  // --------------------------------------------------------------------------
  // [Client Geometry]
  // --------------------------------------------------------------------------

  //! @brief Get widget client geometry.
  FOG_INLINE const RectI& getClientGeometry() const { return _clientGeometry; }

  //! @brief Get widget position relative to parent.
  FOG_INLINE const PointI& getClientPosition() const { return _clientGeometry.getPosition(); }
  //! @brief Get widget size.
  FOG_INLINE const SizeI& getClientSize() const { return _clientGeometry.getSize(); }

  //! @brief Get widget left position, this method is equal to @c left().
  FOG_INLINE int getClientX() const { return _clientGeometry.x; }
  //! @brief Get widget top position, this method is equal to @c top().
  FOG_INLINE int getClientY() const { return _clientGeometry.y; }
  //! @brief Get widget width.
  FOG_INLINE int getClientWidth() const { return _clientGeometry.w; }
  //! @brief Get widget height.
  FOG_INLINE int getClientHeight() const { return _clientGeometry.h; }

  //! @brief Calculate widget geometry from client geometry.
  virtual void calcWidgetSize(SizeI& size) const;

  //! @brief Calculate client geometry from widget geometry.
  //!
  //! Initial position of rect is [0, 0] and initial size is [width, height].
  //! The default implementation is to do nothing, this means leaving size and
  //! position as is (so non-client area is unused).
  virtual void calcClientGeometry(RectI& geometry) const;

  //! @brief Update client geometry and layout.
  //!
  //! This method is called if client geometry need to be updated. This means
  //! sending geometry event and scheduling repaint.
  void updateClientGeometry();

  // WIDGET TODO: Move to .cpp
  FOG_INLINE RectI getClientContentGeometry() const
  {
    RectI ret = _clientGeometry;

    if (ret.getWidth() == 0 && ret.getHeight() == 0)
      return RectI(0, 0, 0, 0);

    ret.setLeft(getContentLeftMargin());
    ret.setTop(getContentTopMargin());
    ret.setWidth(_clientGeometry.getWidth() - getContentRightMargin());
    ret.setHeight(_clientGeometry.getHeight() - getContentBottomMargin());

    return ret;
  }

  // --------------------------------------------------------------------------
  // [Client Origin]
  // --------------------------------------------------------------------------

  //! @brief Get widget origin.
  FOG_INLINE const PointI& getOrigin() const { return _clientOrigin; }

  //! @brief Set widget origin to @a pt.
  void setOrigin(const PointI& pt);

  // --------------------------------------------------------------------------
  // [Translate Coordinates]
  // --------------------------------------------------------------------------

  //! @brief Translate world coordinate @a coord into client (relative to the
  //! widget).
  bool worldToClient(PointI* coord) const;

  //! @brief Translate client coordinate @a coord into world (relative to the
  //! screen).
  bool clientToWorld(PointI* coord) const;

  //! @brief Translate coordinates between two widgets.
  static bool translateCoordinates(Widget* to, Widget* from, PointI* coords);

  // --------------------------------------------------------------------------
  // [Hit Testing]
  // --------------------------------------------------------------------------

  Widget* getChildAt(const PointI& pt, bool recursive = false) const;

  // --------------------------------------------------------------------------
  // [Widget Layout]
  // --------------------------------------------------------------------------

  void invalidateLayout();

  virtual void calculateLayoutHint(LayoutHint& hint);
  virtual bool hasLayoutHeightForWidth() const;
  virtual int getLayoutHeightForWidth(int width) const;

  //! @brief Get widget layout manager.
  FOG_INLINE Layout* getLayout() const { return _layout; }

  //! @brief Set widget layout manager.
  void setLayout(Layout* layout);

  //! @brief Delete widget layout manager (will first disconnects children).
  void deleteLayout();

  //! @brief Take widget layout manager (will not disconnect children).
  Layout* takeLayout();

  // TODO GUI: Purpose? Move to .cpp.
  FOG_INLINE virtual bool isEmpty() const { return !isVisible() || _guiWindow; }

  virtual uint32_t getLayoutExpandingDirections() const;

  //methods for doing real geometry changes
  //don't dispatch geometry events because updates will be done AFTER finishing all
  //geometry changes
  virtual void setLayoutGeometry(const RectI& TODO_give_me_a_name);

  // --------------------------------------------------------------------------
  // [Layout Policy]
  // --------------------------------------------------------------------------

  virtual LayoutPolicy getLayoutPolicy() const;
  virtual void setLayoutPolicy(const LayoutPolicy& policy);

  // --------------------------------------------------------------------------
  // [Layout Height For Width]
  // --------------------------------------------------------------------------

  virtual bool hasHeightForWidth() const;
  virtual int getHeightForWidth(int width) const;

  // --------------------------------------------------------------------------
  // [Layout SizeHint]
  // --------------------------------------------------------------------------

  virtual SizeI getMinimumSizeHint() const;
  virtual SizeI getMaximumSizeHint() const;
  virtual SizeI getSizeHint() const;

  // --------------------------------------------------------------------------
  // [Layout Fixed Size]
  // --------------------------------------------------------------------------

  // --------------------------------------------------------------------------
  // [Layout Minimum And Maximum Size]
  // --------------------------------------------------------------------------

  // WIDGET TODO: Move to .cpp
  FOG_INLINE void checkMinMaxBlock()
  { if (!_extra) _extra = fog_new ExtendedData(); }

  bool checkMinimumSize(int width, int height);
  bool checkMaximumSize(int width, int height);
  void setMinimumSize(const SizeI& minSize);
  void setMaximumSize(const SizeI& minSize);

  // WIDGET TODO: Move to .cpp
  FOG_INLINE void setMinimumHeight(int height)
  {
    int width = hasMinimumHeight()? _extra->_minwidth : -1;
    setMinimumSize(SizeI(width,height));
  }
  FOG_INLINE void setMinimumWidth(int width)
  {
    int height = hasMinimumHeight()? _extra->_maxheight : -1;
    setMinimumSize(SizeI(width,height));
  }
  FOG_INLINE void setMaximumHeight(int height)
  {
    int width = hasMaximumHeight()? _extra->_minwidth : -1;
    setMaximumSize(SizeI(width,height));
  }
  void setMaximumWidth(int width)
  {
    int height = hasMaximumHeight()? _extra->_maxheight : -1;
    setMaximumSize(SizeI(width,height));
  }

  FOG_INLINE bool hasMinimumHeight() const { return _minset & MIN_HEIGHT_IS_SET; }
  FOG_INLINE bool hasMaximumHeight() const { return _maxset & MAX_HEIGHT_IS_SET; }
  FOG_INLINE bool hasMinimumWidth() const { return _minset & MIN_WIDTH_IS_SET; }
  FOG_INLINE bool hasMaximumWidth() const { return _maxset & MAX_WIDTH_IS_SET; }
  FOG_INLINE int getMinimumHeight() const { return getMinimumSize().getHeight(); }
  FOG_INLINE int getMaximumHeight() const { return getMaximumSize().getHeight(); }
  FOG_INLINE int getMinimumWidth() const { return getMinimumSize().getWidth(); }
  FOG_INLINE int getMaximumWidth() const { return getMaximumSize().getWidth(); }

  FOG_INLINE SizeI getMinimumSize() const { return _extra ? SizeI(_extra->_minwidth, _extra->_minheight) : SizeI(WIDGET_MIN_SIZE, WIDGET_MIN_SIZE); }
  FOG_INLINE SizeI getMaximumSize() const { return _extra ? SizeI(_extra->_maxwidth, _extra->_maxheight) : SizeI(WIDGET_MAX_SIZE, WIDGET_MAX_SIZE); }

  // --------------------------------------------------------------------------
  // [Layout State]
  // --------------------------------------------------------------------------

  // --------------------------------------------------------------------------
  // [Widget State]
  // --------------------------------------------------------------------------

  //! @brief Get widget state, see @c WIDGET_STATE.
  FOG_INLINE uint32_t getState() const { return _state; }
  //! @brief Get whether widget is enabled.
  FOG_INLINE bool isEnabled() const { return _state == WIDGET_ENABLED; }
  //! @brief Get whether widget is enabled to parent.
  FOG_INLINE bool isEnabledToParent() const { return _state != WIDGET_DISABLED; }

  //! @brief Set widget state to @a val.
  void setEnabled(bool val = true);

  //! @brief Set widget state to @c WIDGET_ENABLED.
  FOG_INLINE void enable() { setEnabled(true); }
  //! @brief Set widget state to @c WIDGET_DISABLED.
  FOG_INLINE void disable() { setEnabled(false); }

  // --------------------------------------------------------------------------
  // [Widget Visibility]
  // --------------------------------------------------------------------------

  //! @brief Returns widget visibility, see @c Visibility_Enum.
  FOG_INLINE uint32_t getVisibility() const { return _visibility; }

  //! @brief Get whether widget is visible.
  FOG_INLINE bool isVisible() const { return _visibility >= WIDGET_VISIBLE; }
  //! @brief Get whether widget is visible to parent.
  FOG_INLINE bool isVisibleToParent() const { return _state != WIDGET_HIDDEN && _state != WIDGET_VISIBLE_MINIMIZED; }

  // WIDGET TODO: Widget visibility is wrong.

  //! @brief Set widget visibility to @a val.
  void setVisible(uint32_t val = WIDGET_VISIBLE);

  //! @brief Show widget (set it's visibility to WIDGET_VISIBLE).
  FOG_INLINE void show(uint32_t type = WIDGET_VISIBLE) { setVisible(type); }
  //! @brief Show widget (set it's visibility to WIDGET_VISIBLE).
  void showModal(GuiWindow* owner);

  //! @brief Hide widget (set it's visibility to WIDGET_HIDDEN).
  FOG_INLINE void hide() { setVisible(WIDGET_HIDDEN); }

  //! @brief Show widget as FullScreen (set it's visibility to WIDGET_VISIBLE_FULLSCREEN).
  FOG_INLINE void showFullScreen() { setVisible(WIDGET_VISIBLE_FULLSCREEN); }
  //! @brief Show widget maximized (set it's visibility to WIDGET_VISIBLE_MAXIMIZED).
  FOG_INLINE void showMaximized() { setVisible(WIDGET_VISIBLE_MAXIMIZED); }
  //! @brief Show widget minimized (set it's visibility to WIDGET_VISIBLE_MINIMIZED).
  FOG_INLINE void showMinimized() { setVisible(WIDGET_VISIBLE_MINIMIZED); }
  //! @brief Show widget (set it's visibility to WIDGET_VISIBLE).
  FOG_INLINE void showNormal() { setVisible(WIDGET_VISIBLE); }

  //! @brief returns true if the widget is currently shown as full screen
  FOG_INLINE bool isFullScreen() const { return (_visibility == WIDGET_VISIBLE_FULLSCREEN); }
  //! @brief returns true if the widget is currently maximized
  FOG_INLINE bool isMaximized() const { return (_visibility == WIDGET_VISIBLE_MAXIMIZED); }
  //! @brief returns true if the widget is currently minimized
  FOG_INLINE bool isMinimized() const {return ( _visibility == WIDGET_VISIBLE_MINIMIZED); }

  // --------------------------------------------------------------------------
  // [Widget Window Style]
  // --------------------------------------------------------------------------

  //! @brief returns the current window flags of the widget
  FOG_INLINE uint32_t getWindowFlags() const { return _windowFlags; }
  //! @brief set the current window flags of the widget (overwrites existing)
  void setWindowFlags(uint32_t flags);

  //! @brief set WindowFlags without notifying the window manager!
  //! Make sure you know what you are doing!
  FOG_INLINE void overrideWindowFlags(uint32_t flags) { _windowFlags = flags; }

  //! @brief returns the current window hints of the widget
  FOG_INLINE uint32_t getWindowHints() const { return _windowFlags & WINDOW_HINTS_MASK; }
  //! @brief set the current window hints of the widget (but keep window_type!)
  void setWindowHints(uint32_t flags);

  //! @brief returns true if the widget is currently allowed to drag
  FOG_INLINE bool isDragAble() const { return (_windowFlags & WINDOW_DRAGABLE); }
  //! @brief sets the permission to drag the window
  // WIDGET TODO: Why there is update=true?
  void setDragAble(bool drag, bool update = true);

  //! @brief returns true if the widget is currently allowed to resize
  FOG_INLINE bool isResizeAble() const { return (_windowFlags & WINDOW_FIXED_SIZE) == 0; }
  //! @brief sets the permission to resize the window
  // WIDGET TODO: Why there is update=true?
  void setResizeAble(bool resize, bool update = true);

  //! @brief returns true if the widget lays on top of the other windows
  FOG_INLINE bool isAlwaysOnTop() { return (_windowFlags & WINDOW_ALWAYS_ON_TOP) != 0; }
  //! @brief sets the ON_TOP flag of the window
  void setAlwaysOnTop(bool ontop);

  FOG_INLINE bool isPopUpWindow() const { return (((_windowFlags & WINDOW_POPUP) != 0) || ((_windowFlags & WINDOW_INLINE_POPUP) != 0)); }

  // --------------------------------------------------------------------------
  // [Transparency]
  // --------------------------------------------------------------------------

  FOG_INLINE float getTransparency() const { return _transparency; }

  void setTransparency(float val);

  // --------------------------------------------------------------------------
  // [Widget Orientation]
  // --------------------------------------------------------------------------

  //! @brief Returns widget orientation, see @c OrientationEnum.
  FOG_INLINE uint32_t getOrientation() const { return _orientation; }

  //! @brief Sets widget orientation.
  void setOrientation(uint32_t val);

  // --------------------------------------------------------------------------
  // [Caret]
  // --------------------------------------------------------------------------

  // bool showCaret();
  // bool showCaret(const CaretProperties& properties);
  // bool hideCaret();

  // --------------------------------------------------------------------------
  // [Tab Order]
  // --------------------------------------------------------------------------

  //! @brief Returns widget tab order.
  FOG_INLINE int getTabOrder() const { return _tabOrder; }

  //! @brief Sets widget tab order.
  void setTabOrder(int tabOrder);

  // --------------------------------------------------------------------------
  // [Focus]
  // --------------------------------------------------------------------------

  FOG_INLINE uint32_t getFocusPolicy() const { return _focusPolicy; }

  void setFocusPolicy(uint32_t val);

  FOG_INLINE bool hasFocus() const { return _hasFocus; }

  virtual Widget* getFocusableWidget(int focusable);

  //! @brief Takes client focus for this widget.
  void takeFocus(uint32_t reason = FOCUS_REASON_NONE);

  void giveFocusNext(uint32_t reason = FOCUS_REASON_NONE);
  void giveFocusPrevious(uint32_t reason = FOCUS_REASON_NONE);

  Widget* _findFocus() const;

  // --------------------------------------------------------------------------
  // [Font]
  // --------------------------------------------------------------------------

  //! @brief Get widget font.
  FOG_INLINE Font getFont() const { return _font; }

  //! @brief Set widget font.
  void setFont(const Font& font);

  // --------------------------------------------------------------------------
  // [Update]
  // --------------------------------------------------------------------------

  //! @brief Update widget (see @c WIDGET_UPDATE_FLAGS).
  void update(uint32_t updateFlags);

  // --------------------------------------------------------------------------
  // [Painting]
  // --------------------------------------------------------------------------

  //! @brief Get paint hints, see @c WIDGET_PAINT_MODE.
  //!
  //! Paint hints are used to optimize widget repainting process. Each widget
  //! must be paint when it's displayed on the screen. There are widgets that
  //! usually repaint the entire area (@c TextBox, @c TextArea, @c ToolBar, etc)
  //! and there are widgets that in most cases need the background provided by
  //! parent (very good example is @c Label).
  //!
  //! Use @c getPaintHint() to tell Fog-Gui how the widget will be updated.
  //! Default return value is @c WIDGET_PAINT_SCREEN which means that entire
  //! widget will be updated (result is not based on parent content). If you
  //! need to draw rounded corners or to content paint previously by parent,
  //! use @c WIDGET_PAINT_PROPAGATED hint.
  //!
  //! If you return @c WIDGET_PAINT_PROPAGATED then the gui engine always call
  //! the @c Widget::getPropagatedRegion() method to get which region will be
  //! propagated. You don't need to reimplement this method (it will return the
  //! whole widget region by default). If you want to draw only rounded corners
  //! or something that will overpaint only very small area, you can reimplement
  //! this method to optimize painting process.
  virtual uint32_t getPaintHint() const;

  //! @brief Get region that will be propagated from parent to the @c onPaint()
  //! event handler.
  //!
  //! Default action is to return the entire widget region. If an error is
  //! returned then gui engine will generate the whole region automatically
  //! (but never return just error, it will be probably logged to stderr).
  virtual err_t getPropagatedRegion(Region* dst) const;

  // --------------------------------------------------------------------------
  // [Event Handlers]
  // --------------------------------------------------------------------------

  //! @brief State event handler.
  virtual void onState(StateEvent* e);

  //! @brief Visibility event handler.
  virtual void onVisibility(VisibilityEvent* e);

  //! @brief Configure event handler.
  virtual void onGeometry(GeometryEvent* e);

  //! @brief Focus in / out event handler.
  virtual void onFocus(FocusEvent* e);

  //! @brief Keyboard press / release event handler.
  virtual void onKey(KeyEvent* e);

  //! @brief Non-client area mouse event handler.
  virtual void onNcMouse(MouseEvent* e);

  //! @brief Mouse event handler.
  virtual void onMouse(MouseEvent* e);

  //! @brief Click event handler.
  virtual void onClick(MouseEvent* e);

  //! @brief Double click event handler.
  virtual void onDoubleClick(MouseEvent* e);

  //! @brief Wheel event handler.
  virtual void onWheel(MouseEvent* e);

  //! @brief Selection event handler.
  virtual void onSelection(SelectionEvent* e);

  //! @brief Non-client area paint handler.
  virtual void onNcPaint(PaintEvent* e);

  //! @brief Client area paint handler.
  virtual void onPaint(PaintEvent* e);

  //! @brief Close event handler.
  virtual void onClose(CloseEvent* e);

  //! @brief Theme changed event handler.
  virtual void onThemeChange(ThemeEvent* e);

  //! @brief @c LayoutItem event handler.
  virtual void onLayout(LayoutEvent* e);

  // --------------------------------------------------------------------------
  // [Event Map]
  // --------------------------------------------------------------------------

  FOG_EVENT_BEGIN()
    FOG_EVENT_DEF(EVENT_ENABLE              , onState           , StateEvent     , OVERRIDE)
    FOG_EVENT_DEF(EVENT_DISABLE             , onState           , StateEvent     , OVERRIDE)
    FOG_EVENT_DEF(EVENT_DISABLE_BY_PARENT   , onState           , StateEvent     , OVERRIDE)

    FOG_EVENT_DEF(EVENT_SHOW                , onVisibility      , VisibilityEvent, OVERRIDE)
    FOG_EVENT_DEF(EVENT_HIDE                , onVisibility      , VisibilityEvent, OVERRIDE)
    FOG_EVENT_DEF(EVENT_HIDE_BY_PARENT      , onVisibility      , VisibilityEvent, OVERRIDE)

    // TODO: Move, remove, do something...
    FOG_EVENT_DEF(EVENT_SHOW_FULLSCREEN     , onVisibility      , VisibilityEvent, OVERRIDE)
    FOG_EVENT_DEF(EVENT_SHOW_MAXIMIZE       , onVisibility      , VisibilityEvent, OVERRIDE)
    FOG_EVENT_DEF(EVENT_SHOW_MINIMIZE       , onVisibility      , VisibilityEvent, OVERRIDE)

    FOG_EVENT_DEF(EVENT_GEOMETRY            , onGeometry        , GeometryEvent  , OVERRIDE)

    FOG_EVENT_DEF(EVENT_FOCUS_IN            , onFocus           , FocusEvent     , OVERRIDE)
    FOG_EVENT_DEF(EVENT_FOCUS_OUT           , onFocus           , FocusEvent     , OVERRIDE)

    FOG_EVENT_DEF(EVENT_KEY_PRESS           , onKey             , KeyEvent       , OVERRIDE)
    FOG_EVENT_DEF(EVENT_KEY_RELEASE         , onKey             , KeyEvent       , OVERRIDE)

    FOG_EVENT_DEF(EVENT_NCMOUSE_IN          , onNcMouse         , MouseEvent     , OVERRIDE)
    FOG_EVENT_DEF(EVENT_NCMOUSE_OUT         , onNcMouse         , MouseEvent     , OVERRIDE)
    FOG_EVENT_DEF(EVENT_NCMOUSE_MOVE        , onNcMouse         , MouseEvent     , OVERRIDE)
    FOG_EVENT_DEF(EVENT_NCMOUSE_PRESS       , onNcMouse         , MouseEvent     , OVERRIDE)
    FOG_EVENT_DEF(EVENT_NCMOUSE_RELEASE     , onNcMouse         , MouseEvent     , OVERRIDE)

    FOG_EVENT_DEF(EVENT_MOUSE_IN            , onMouse           , MouseEvent     , OVERRIDE)
    FOG_EVENT_DEF(EVENT_MOUSE_OUT           , onMouse           , MouseEvent     , OVERRIDE)
    FOG_EVENT_DEF(EVENT_MOUSE_MOVE          , onMouse           , MouseEvent     , OVERRIDE)
    FOG_EVENT_DEF(EVENT_MOUSE_PRESS         , onMouse           , MouseEvent     , OVERRIDE)
    FOG_EVENT_DEF(EVENT_MOUSE_RELEASE       , onMouse           , MouseEvent     , OVERRIDE)

    FOG_EVENT_DEF(EVENT_CLICK               , onClick           , MouseEvent     , OVERRIDE)
    FOG_EVENT_DEF(EVENT_DOUBLE_CLICK        , onDoubleClick     , MouseEvent     , OVERRIDE)
    FOG_EVENT_DEF(EVENT_WHEEL               , onWheel           , MouseEvent     , OVERRIDE)

    FOG_EVENT_DEF(EVENT_CLEAR_SELECTION     , onSelection       , SelectionEvent , OVERRIDE)
    FOG_EVENT_DEF(EVENT_SELECTION_REQUIRED  , onSelection       , SelectionEvent , OVERRIDE)

    FOG_EVENT_DEF(EVENT_NCPAINT             , onNcPaint         , PaintEvent     , OVERRIDE)
    FOG_EVENT_DEF(EVENT_PAINT               , onPaint           , PaintEvent     , OVERRIDE)

    FOG_EVENT_DEF(EVENT_CLOSE               , onClose           , CloseEvent     , OVERRIDE)

    FOG_EVENT_DEF(EVENT_THEME               , onThemeChange     , ThemeEvent     , OVERRIDE)

    FOG_EVENT_DEF(EVENT_LAYOUT_SET          , onLayout          , LayoutEvent    , OVERRIDE)
    FOG_EVENT_DEF(EVENT_LAYOUT_REMOVE       , onLayout          , LayoutEvent    , OVERRIDE)
  FOG_EVENT_END()

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

protected:
  //! @brief Widget parent link or @c NULL.
  Widget* _parentWidget;

  // TODO: Stefan, what is purpose of this member and why it's here, thanks;)
  GuiWindow* _owner;

  //! @brief GuiWindow (top-level) associated with this widget.
  GuiWindow* _guiWindow;

  //! @brief Widget main geometry (geometry relative to widget parent or screen).
  RectI _widgetGeometry;
  //! @brief Widget client geometry (geometry where all children are placed).
  RectI _clientGeometry;
  //! @brief Client origin.
  PointI _clientOrigin;

  // GUI TODO: Move to GuiWindow.
  struct FullScreenData
  {
    //! @brief Main geometry for restoration from fullscreen mode
    RectI _restoregeometry;
    //! @brief Window Style for restoration of fullscreen
    uint32_t _restorewindowFlags;
    float _restoretransparency;
  }* _fullscreendata;

  // GUI TODO: Move...
  struct ExtendedData
  {
    ExtendedData() : _maxwidth(WIDGET_MAX_SIZE), _maxheight(WIDGET_MAX_SIZE), _minwidth(WIDGET_MIN_SIZE), _minheight(WIDGET_MIN_SIZE) {}
    int _maxwidth;
    int _maxheight;
    int _minwidth;
    int _minheight;
  }* _extra;

  //! @brief Will set/unset a window flag and update the window if specified
  void changeFlag(uint32_t flag, bool set, bool update);

  //! @brief Font (used to draw text in widget).
  Font _font;

  //! @brief Layout.
  Layout* _layout;

  //! @brief Layout policy.
  // LAYOUT TODO: move into LayoutItem??
  LayoutPolicy _layoutPolicy;

  //! @brief Tab order.
  int _tabOrder;

  //! @brief Widget transparency (0.0 to 1.0).
  float _transparency;

  //! @brief Link to child that was last focus.
  Widget* _lastFocus;
  Widget* _focusLink;

  //! @brief Update flags.
  uint32_t _uflags;
  //! @brief Window Style.
  uint32_t _windowFlags;

  //! @brief Whether the minwidth / minheight is set.
  uint32_t _minset : 2;
  //! @brief Whether the maxwidth / maxheight is set.
  uint32_t _maxset : 2;
  //! @brief Focus.
  uint32_t _hasFocus : 1;
  //! @brief Widget orientation.
  uint32_t _orientation : 1;
  //! @brief Widget state.
  uint32_t _state : 2;

  //! @brief Widget visibility.
  uint32_t _visibility : 4;
  //! @brief Widget focus policy.
  uint32_t _focusPolicy : 4;
  //! @brief Whether the widget has non-client area.
  uint32_t _hasNcArea : 1;

  //! @brief Reserved for future use.
  uint32_t _reserved : 15;

  uint32_t _widgetflags;

private:
  friend struct Application;
  friend struct Layout;
  friend struct GuiEngine;
  friend struct GuiWindow;
  friend struct Window;

  FOG_DISABLE_COPY(Widget)
};

//! @}

} // Fog namespace

// [Guard]
#endif // _FOG_GUI_WIDGET_WIDGET_H
