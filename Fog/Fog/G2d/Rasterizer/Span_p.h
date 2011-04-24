// [Fog-G2d]
//
// [License]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_G2D_RASTERIZER_SPAN_P_H
#define _FOG_G2D_RASTERIZER_SPAN_P_H

// [Dependencies]
#include <Fog/Core/Global/Assert.h>
#include <Fog/G2d/Global/Constants.h>

namespace Fog {

//! @addtogroup Fog_G2d_Rasterizer
//! @{

// ============================================================================
// [Constants]
// ============================================================================

//! @brief Helper constant to determine how many constant pixels are
//! profitable for pure CMask span instance. Minimum is 1, but recommended
//! are 4 or more (it depends to available SIMD instructions per platform).
enum { SPAN_C_THRESHOLD = 4 };

// ============================================================================
// [Fog::Span]
// ============================================================================

#include <Fog/Core/Pack/PackByte.h>
//! @internal
//!
//! @brief Span is small element that contains position, length, CMask (const
//! mask) or VMask (variant mask).
//!
//! Spans are always related with scanline, so information about Y position is
//! not part of the span. The const mask is encoded in mask pointer so you should
//! always check if span is CMask (@c isConst() method) or vmask (@c isVariant()
//! method). There are asserts so you should be warned that you are using span
//! incorrectly.
//!
//! Note that you can base another class on @c Span to extend its
//! functionality. The core idea is that @c Span is used across the API so you
//! don't need to define more classes to work with spans.
//!
//! @sa @c Span8.
struct FOG_NO_EXPORT Span
{
  // --------------------------------------------------------------------------
  // [Consistency]
  // --------------------------------------------------------------------------

  //! @brief Whether the span is valid (used by asserts).
  FOG_INLINE bool isValid() const { return (_x0 < _x1) && (_mask != NULL); }

  // --------------------------------------------------------------------------
  // [Position / Type]
  // --------------------------------------------------------------------------

  //! @brief Get span type.
  FOG_INLINE uint getType() const { return _type; }

  //! @brief Get span start position, inclusive (x0).
  FOG_INLINE int getX0() const { return (int)_x0; }
  //! @brief Get span end position, exclusive (x1).
  FOG_INLINE int getX1() const { return (int)_x1; }

  //! @brief Get span length, computed as <code>x1 - x0</code>.
  FOG_INLINE int getLength() const { return (int)(_x1 - _x0); }

  //! @brief Set span type.
  FOG_INLINE void setType(uint type)
  {
    FOG_ASSERT(type < SPAN_COUNT);
    _type = type;
  }

  //! @brief Set span start position.
  FOG_INLINE void setX0(int x0)
  {
    FOG_ASSERT(x0 >= 0);
    _x0 = (uint32_t)(uint)x0;
  }

  //! @brief Set span end position.
  FOG_INLINE void setX1(int x1)
  {
    FOG_ASSERT(x1 >= 0);
    _x1 = (uint32_t)(uint)x1;
  }

  //! @brief Set span start/end position.
  FOG_INLINE void setPosition(int x0, int x1)
  {
    // Disallow invalid position.
    FOG_ASSERT(x0 >= 0);
    FOG_ASSERT(x1 >= 0);
    FOG_ASSERT(x0 < x1);

    _x0 = x0;
    _x1 = x1;
  }

  //! @brief Set span start/end position and type.
  FOG_INLINE void setPositionAndType(int x0, int x1, uint type)
  {
    // Disallow invalid position.
    FOG_ASSERT(x0 >= 0);
    FOG_ASSERT(x1 >= 0);
    FOG_ASSERT(x0 < x1);

    // I hope that compiler can optimize such case (setting x1 with type).
    _x0 = (uint32_t)(uint)x0;
    _x1 = (uint32_t)(uint)x1;
    _type = type;
  }

  // --------------------------------------------------------------------------
  // [Common Interface]
  // --------------------------------------------------------------------------

  //! @brief Get whether the span is a const-mask.
  FOG_INLINE bool isConst() const { return _type < SPAN_V_BEGIN; }
  //! @brief Get whether the span is a variant-mask.
  FOG_INLINE bool isVariant() const { return _type >= SPAN_V_BEGIN; }

  //! @brief Get whether the span is a A8-Glyph.
  FOG_INLINE bool isA8Glyph() const { return _type == SPAN_A8_GLYPH; }
  //! @brief Get whether the span is a AX-Glyph.
  FOG_INLINE bool isAXGlyph() const { return _type == SPAN_AX_GLYPH; }
  //! @brief Get whether the span is a AX-Extra.
  FOG_INLINE bool isAXExtra() const { return _type == SPAN_AX_EXTRA; }

  //! @brief Get whether the span is a ARGB32-Glyph.
  FOG_INLINE bool isArgb32Glyph() const { return _type == SPAN_ARGB32_GLYPH; }
  //! @brief Get whether the span is a ARGBXX-Glyph.
  FOG_INLINE bool isArgbXXGlyph() const { return _type == SPAN_ARGBXX_GLYPH; }

  // --------------------------------------------------------------------------
  // [Generic-Mask Interface]
  // --------------------------------------------------------------------------

  //! @brief Get the generic mask pointer.
  FOG_INLINE void* getGenericMask() const { return _mask; }

  //! @brief Set the generic mask pointer to @a mask.
  FOG_INLINE void setGenericMask(void* mask) { _mask = mask; }

  // --------------------------------------------------------------------------
  // [Variant-Mask Interface]
  // --------------------------------------------------------------------------

  FOG_INLINE uint8_t* getVariantMask() const
  {
    FOG_ASSERT(isVariant());
    return reinterpret_cast<uint8_t*>(_mask);
  }

  FOG_INLINE void setVariantMask(uint8_t* mask)
  {
    FOG_ASSERT(isVariant());
    _mask = reinterpret_cast<void*>(mask);
  }

  // --------------------------------------------------------------------------
  // [Next]
  // --------------------------------------------------------------------------

  //! @brief Get the next span.
  FOG_INLINE Span* getNext() const { return _next; }
  //! @brief Set the next span.
  FOG_INLINE void setNext(Span* next) { _next = next; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Start of span (first valid pixel, inclusive).
  uint32_t _x0 : 29;
  //! @brief Type of span. See SPAN.
  uint32_t _type : 3;
  //! @brief End of span (last valid pixel, exclusive).
  uint32_t _x1;

  union
  {
    //! @brief CMask value or pointer to VMask.
    //!
    //! Use CMask/VMask getters & setters to access or modify this value.
    void* _mask;
    sysuint_t _mask_uint;
  };

  //! @brief Pointer to the next span (or @c NULL).
  Span* _next;
};
#include <Fog/Core/Pack/PackRestore.h>

// ============================================================================
// [Fog::Span8]
// ============================================================================

#include <Fog/Core/Pack/PackByte.h>
//! @internal
//!
//! @brief Span that is used by rendering to 8-bit image formats.
//!
//! Image formats:
//! - @c IMAGE_FORMAT_PRGB32.
//! - @c IMAGE_FORMAT_XRGB32.
//! - @c IMAGE_FORMAT_RGB24.
//! - @c IMAGE_FORMAT_A8.
struct FOG_NO_EXPORT Span8 : public Span
{
  // --------------------------------------------------------------------------
  // [Const-Mask Interface]
  // --------------------------------------------------------------------------

  //! @brief Get the const value.
  //!
  //! It's allowed to call this method only when @c isConst() returns @c true.
  FOG_INLINE uint32_t getConstMask() const
  {
    // Never call getConstMask() on VMask span.
    FOG_ASSERT(isConst());

    return (uint32_t)_mask_uint;
  }

  FOG_INLINE bool isConstMaskOpaque() const
  {
    // Never call getConstMask() on VMask span.
    FOG_ASSERT(isConst());

    return _mask_uint == 0x100;
  }

  //! @brief Set const mask value.
  FOG_INLINE void setConstMask(uint32_t mask)
  {
    // Never call setConstMask() on the VMask span.
    FOG_ASSERT(isConst());

    _mask_uint = (sysuint_t)mask;
  }

  // --------------------------------------------------------------------------
  // [A8-Glyph Interface]
  // --------------------------------------------------------------------------

  FOG_INLINE uint8_t* getA8Glyph() const
  {
    FOG_ASSERT(getType() == SPAN_A8_GLYPH || getType() == SPAN_AX_GLYPH);
    return reinterpret_cast<uint8_t*>(_mask);
  }

  FOG_INLINE void setA8Glyph(uint8_t* mask)
  {
    FOG_ASSERT(getType() == SPAN_A8_GLYPH || getType() == SPAN_AX_GLYPH);
    _mask = reinterpret_cast<uint8_t*>(mask);
  }

  // --------------------------------------------------------------------------
  // [A8-Extra Interface]
  // --------------------------------------------------------------------------

  FOG_INLINE uint8_t* getA8Extra() const
  {
    FOG_ASSERT(getType() == SPAN_AX_EXTRA);
    return reinterpret_cast<uint8_t*>(_mask);
  }

  FOG_INLINE void setA8Extra(uint8_t* mask)
  {
    FOG_ASSERT(getType() == SPAN_AX_EXTRA);
    _mask = reinterpret_cast<uint8_t*>(mask);
  }

  // --------------------------------------------------------------------------
  // [ARGB32-Glyph Interface]
  // --------------------------------------------------------------------------

  FOG_INLINE uint8_t* getARGB32Glyph() const
  {
    FOG_ASSERT(getType() == SPAN_ARGB32_GLYPH || getType() == SPAN_ARGBXX_GLYPH);
    return reinterpret_cast<uint8_t*>(_mask);
  }

  FOG_INLINE void setARGB32Glyph(uint8_t* mask)
  {
    FOG_ASSERT(getType() == SPAN_ARGB32_GLYPH || getType() == SPAN_ARGBXX_GLYPH);
    _mask = reinterpret_cast<uint8_t*>(mask);
  }

  // --------------------------------------------------------------------------
  // [Next]
  // --------------------------------------------------------------------------

  //! @brief Get the next span.
  FOG_INLINE Span8* getNext() const
  {
    return reinterpret_cast<Span8*>(_next);
  }

  // --------------------------------------------------------------------------
  // [Statics]
  // --------------------------------------------------------------------------

  static FOG_INLINE int getMaskAdvance(uint32_t type, int width)
  {
    // ${SPAN:BEGIN}
    static const int advanceData[] =
    {
      /* 00 - SPAN_C            */ 0,
      /* 01 - SPAN_A8_GLYPH     */ 1,
      /* 02 - SPAN_AX_GLYPH     */ 1,
      /* 03 - SPAN_AX_EXTRA     */ 2,
      /* 04 - SPAN_ARGB32_GLYPH */ 4,
      /* 05 - SPAN_ARGBXX_GLYPH */ 4,
      /* 06 - ...               */ 0
    };
    // ${SPAN:END}

    return width * advanceData[type];
  }

  static FOG_INLINE bool isConstMaskPointer(const uint8_t* mask)
  {
    return (sysuint_t)mask <= (sysuint_t)0x100;
  }

  static FOG_INLINE bool isVariantMaskPointer(const uint8_t* mask)
  {
    return (sysuint_t)mask > (sysuint_t)0x100;
  }

  static FOG_INLINE uint32_t getConstMaskFromPointer(const uint8_t* mask)
  {
    return (uint32_t)(sysuint_t)mask;
  }

  static FOG_INLINE uint8_t* getPointerFromConstMask(uint32_t mask)
  {
    return (uint8_t*)(void*)mask;
  }
};
#include <Fog/Core/Pack/PackRestore.h>

// ============================================================================
// [Fog::SpanExt8]
// ============================================================================

#include <Fog/Core/Pack/PackByte.h>
//! @internal
//!
//! @brief @c Span8 extended by a @c data pointer.
struct FOG_NO_EXPORT SpanExt8 : public Span8
{
  // --------------------------------------------------------------------------
  // [Data]
  // --------------------------------------------------------------------------

  //! @brief Get the @c SpanExt8 data.
  FOG_INLINE uint8_t* getData() const
  {
    return _data;
  }

  //! @brief Set the @c SpanExt8 data.
  FOG_INLINE void setData(uint8_t* data)
  {
    _data = data;
  }

  // --------------------------------------------------------------------------
  // [Next]
  // --------------------------------------------------------------------------

  //! @brief Get the next span.
  FOG_INLINE SpanExt8* getNext() const
  {
    return reinterpret_cast<SpanExt8*>(_next);
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Data pointer, used by patterns to store pointer to a fetched pixels.
  uint8_t* _data;
};
#include <Fog/Core/Pack/PackRestore.h>

// ============================================================================
// [Fog::Span16]
// ============================================================================

#include <Fog/Core/Pack/PackByte.h>
//! @internal
//!
//! @brief Span that is used by rendering to 16-bit image formats.
//!
//! Image formats:
//! - @c IMAGE_FORMAT_ARGB64.
//! - @c IMAGE_FORMAT_PRGB64.
//! - @c IMAGE_FORMAT_RGB48.
//! - @c IMAGE_FORMAT_A16.
struct FOG_NO_EXPORT Span16 : public Span
{
  // --------------------------------------------------------------------------
  // [Const-Mask Interface]
  // --------------------------------------------------------------------------

  //! @brief Get the const value.
  //!
  //! It's allowed to call this method only when @c isConst() returns @c true.
  FOG_INLINE uint32_t getConstMask() const
  {
    // Never call getCMask() on VMask span.
    FOG_ASSERT(isConst());

    return (uint32_t)_mask_uint;
  }

  FOG_INLINE bool isConstMaskOpaque() const
  {
    // Never call getConstMask() on VMask span.
    FOG_ASSERT(isConst());

    return _mask_uint == 0x10000;
  }

  //! @brief Set const mask value.
  FOG_INLINE void setConstMask(uint32_t mask)
  {
    // Never call setCMask() on the VMask span.
    FOG_ASSERT(isConst());

    _mask_uint = (sysuint_t)mask;
  }

  // --------------------------------------------------------------------------
  // [A8-Glyph Interface]
  // --------------------------------------------------------------------------

  FOG_INLINE uint8_t* getA8Glyph() const
  {
    FOG_ASSERT(getType() == SPAN_A8_GLYPH);
    return reinterpret_cast<uint8_t*>(_mask);
  }

  FOG_INLINE void setA8Glyph(uint8_t* mask)
  {
    FOG_ASSERT(getType() == SPAN_A8_GLYPH);
    _mask = reinterpret_cast<uint8_t*>(mask);
  }

  // --------------------------------------------------------------------------
  // [A16-Glyph Interface]
  // --------------------------------------------------------------------------

  FOG_INLINE uint8_t* getA16Glyph() const
  {
    FOG_ASSERT(getType() == SPAN_AX_GLYPH);
    return reinterpret_cast<uint8_t*>(_mask);
  }

  FOG_INLINE void setA16Glyph(uint8_t* mask)
  {
    FOG_ASSERT(getType() == SPAN_AX_GLYPH);
    _mask = reinterpret_cast<uint8_t*>(mask);
  }

  // --------------------------------------------------------------------------
  // [A16-Extra Interface]
  // --------------------------------------------------------------------------

  FOG_INLINE uint8_t* getA16Extra() const
  {
    FOG_ASSERT(getType() == SPAN_AX_EXTRA);
    return reinterpret_cast<uint8_t*>(_mask);
  }

  FOG_INLINE void setA16Extra(uint8_t* mask)
  {
    FOG_ASSERT(getType() == SPAN_AX_EXTRA);
    _mask = reinterpret_cast<uint8_t*>(mask);
  }

  // --------------------------------------------------------------------------
  // [ARGB32-Glyph Interface]
  // --------------------------------------------------------------------------

  FOG_INLINE uint8_t* getARGB32Glyph() const
  {
    FOG_ASSERT(getType() == SPAN_ARGB32_GLYPH);
    return reinterpret_cast<uint8_t*>(_mask);
  }

  FOG_INLINE void setARGB32Glyph(uint8_t* mask)
  {
    FOG_ASSERT(getType() == SPAN_ARGB32_GLYPH);
    _mask = reinterpret_cast<uint8_t*>(mask);
  }

  // --------------------------------------------------------------------------
  // [ARGB64-Glyph Interface]
  // --------------------------------------------------------------------------

  FOG_INLINE uint8_t* getARGB64Glyph() const
  {
    FOG_ASSERT(getType() == SPAN_ARGBXX_GLYPH);
    return reinterpret_cast<uint8_t*>(_mask);
  }

  FOG_INLINE void setARGB64Glyph(uint8_t* mask)
  {
    FOG_ASSERT(getType() == SPAN_ARGBXX_GLYPH);
    _mask = reinterpret_cast<uint8_t*>(mask);
  }

  // --------------------------------------------------------------------------
  // [Next]
  // --------------------------------------------------------------------------

  //! @brief Get the next span.
  FOG_INLINE Span16* getNext() const
  {
    return reinterpret_cast<Span16*>(_next);
  }

  // --------------------------------------------------------------------------
  // [Statics]
  // --------------------------------------------------------------------------

  static FOG_INLINE int getMaskAdvance(uint32_t type, int width)
  {
    // ${SPAN:BEGIN}
    static const int advanceData[] =
    {
      /* 00 - SPAN_C            */ 0,
      /* 01 - SPAN_A8_GLYPH     */ 1,
      /* 02 - SPAN_AX_GLYPH     */ 2,
      /* 03 - SPAN_AX_EXTRA     */ 4,
      /* 04 - SPAN_ARGB32_GLYPH */ 4,
      /* 05 - SPAN_ARGBXX_GLYPH */ 8,
      /* 06 - ...               */ 0
    };
    // ${SPAN:END}

    return width * advanceData[type];
  }
};
#include <Fog/Core/Pack/PackRestore.h>

// ============================================================================
// [Fog::SpanExt16]
// ============================================================================

#include <Fog/Core/Pack/PackByte.h>
//! @internal
//!
//! @brief @c Span16 extended by a @c data pointer.
struct FOG_NO_EXPORT SpanExt16 : public Span16
{
  // --------------------------------------------------------------------------
  // [Data]
  // --------------------------------------------------------------------------

  //! @brief Get the @c SpanExt16 data.
  FOG_INLINE uint8_t* getData() const
  {
    return _data;
  }

  //! @brief Set the @c SpanExt16 data.
  FOG_INLINE void setData(uint8_t* data)
  {
    _data = data;
  }

  // --------------------------------------------------------------------------
  // [Next]
  // --------------------------------------------------------------------------

  //! @brief Get the next span.
  FOG_INLINE SpanExt16* getNext() const
  {
    return reinterpret_cast<SpanExt16*>(_next);
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Data pointer, used by patterns to store pointer to a fetched pixels.
  uint8_t* _data;
};
#include <Fog/Core/Pack/PackRestore.h>

//! @}

} // Fog namespace

// [Guard]
#endif // _FOG_G2D_RASTERIZER_SPAN_P_H
