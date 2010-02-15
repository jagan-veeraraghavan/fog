// [Fog/Graphics Library - Public API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_IMAGEIO_H
#define _FOG_GRAPHICS_IMAGEIO_H

// [Dependencies]
#include <Fog/Core/List.h>
#include <Fog/Core/Object.h>
#include <Fog/Core/Stream.h>
#include <Fog/Core/String.h>
#include <Fog/Core/Value.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Palette.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {
namespace ImageIO {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct BaseDevice;
struct DecoderDevice;
struct EncoderDevice;
struct Provider;

// ============================================================================
// [Functions]
// ============================================================================

FOG_API bool addProvider(Provider* provider);
FOG_API bool removeProvider(Provider* provider);
FOG_API bool hasProvider(Provider* provider);

FOG_API List<Provider*> getProviders();

FOG_API Provider* getProviderByName(const String& name);
FOG_API Provider* getProviderByExtension(const String& extension);
FOG_API Provider* getProviderByMime(void* mem, sysuint_t len);

FOG_API DecoderDevice* createDecoderByName(const String& name, err_t* err = NULL);
FOG_API DecoderDevice* createDecoderByExtension(const String& extension, err_t* err = NULL);

FOG_API DecoderDevice* createDecoderForFile(const String& fileName, err_t* err = NULL);
FOG_API DecoderDevice* createDecoderForStream(Stream& stream, const String& extension, err_t* err = NULL);

FOG_API EncoderDevice* createEncoderByName(const String& name, err_t* err = NULL);
FOG_API EncoderDevice* createEncoderByExtension(const String& extension, err_t* err = NULL);

// ============================================================================
// [Fog::ImageIO::Provider]
// ============================================================================

//! @brief Image file encoder/decoder provider singleton.
//!
//! Providers are used to create image decoder and encoder devices and
//! to check if image data can be readed by decoder.
//!
//! Befire image decoder is created, often is checked relevance by
//! mime type data (first image file bytes)
struct FOG_API Provider
{
  // [Construction / Destruction]

  //! @brief Image file format Constructor.
  Provider();
  //! @brief Image file format descructor.
  virtual ~Provider();

  // [Features]

  //! @brief File format features.
  struct Features
  {
    // [Devices]

    //! @brief Provider provides decoder.
    uint32_t decoder : 1;
    //! @brief Provider provides encoder.
    uint32_t encoder : 1;

    //! @brief Provider is proxy provider for another library.
    uint32_t proxy : 1;
  };

  // [Members access]

  //! @brief Returns image file format name.
  FOG_INLINE const String& getName() const { return _name; }
  //! @brief Returns image file format extensions.
  FOG_INLINE const List<String>& getExtensions() const { return _extensions; }
  //! @brief Returns image file format features.
  FOG_INLINE const Features& getFeatures() const { return _features; }

  // [Virtuals]

  //! @brief Check mime type for this format and return it's relevance. 
  //!
  //! @note Relevance is number between 0 to 100 in percents. Image
  //! loader will use relevance for decoders.
  virtual uint32_t check(const void* mem, sysuint_t length) = 0;
  //! @overload
  FOG_INLINE uint32_t check(const ByteArray& mem) { return check(mem.getData(), mem.getLength()); }

  virtual EncoderDevice* createEncoder();
  virtual DecoderDevice* createDecoder();

  // [Members]

protected:
  //! @brief Image file format name ("BMP", "JPEG", "PNG", ...) .
  String _name;
  //! @brief Image file format id.
  uint32_t _id;
  //! @brief Image file format extensions ("bmp", "jpg", "jpeg", ...).
  List<String> _extensions;

  //! @brief Image file format features.
  Features _features;

private:
  FOG_DISABLE_COPY(Provider)
};

// ============================================================================
// [Fog::ImageIO::BaseDevice]
// ============================================================================

struct FOG_API BaseDevice : public Object
{
  FOG_DECLARE_OBJECT(BaseDevice, Object)

  // [Construction / Descruction]

  BaseDevice(Provider* provider);
  virtual ~BaseDevice();

  // [Properties]

  virtual err_t getProperty(const ManagedString& name, Value& value) const;
  virtual err_t setProperty(const ManagedString& name, const Value& value);

  // [Members access]

  FOG_INLINE Provider* getProvider() const { return _provider; }
  FOG_INLINE uint32_t getDeviceType() const { return _deviceType; }
  FOG_INLINE uint32_t getFlags() const { return _flags; }

  FOG_INLINE bool isNone() const { return _deviceType == IMAGEIO_DEVICE_NONE; }
  FOG_INLINE bool isEncoder() const { return (_deviceType & IMAGEIO_DEVICE_ENCODER) != 0; }
  FOG_INLINE bool isDecoder() const { return (_deviceType & IMAGEIO_DECIDE_DECODER) != 0; }
  FOG_INLINE bool isProxy() const { return (_deviceType & IMAGEIO_DEVICE_PROXY) != 0; }

  FOG_INLINE uint64_t attachedOffset() const { return _attachedOffset; }
  FOG_INLINE Stream& getStream() { return _stream; }
  FOG_INLINE const Stream& getStream() const { return _stream; }

  FOG_INLINE uint32_t getWidth() const { return _width; }
  FOG_INLINE uint32_t getHeight() const { return _height; }
  FOG_INLINE uint32_t getDepth() const { return _depth; }
  FOG_INLINE uint32_t getPlanes() const { return _planes; }

  FOG_INLINE uint32_t getActualFrame() const { return _actualFrame; }
  FOG_INLINE uint32_t getFramesCount() const { return _framesCount; }

  FOG_INLINE int getFormat() const { return _format; }
  FOG_INLINE const Palette& getPalette() const { return _palette; }
  FOG_INLINE const ByteArray& getComment() const { return _comment; }

  // [Progress]

  FOG_INLINE float getProgress() const { return _progress; }

  //! @brief Update progress to @a value.
  void updateProgress(float value);
  //! @overload
  void updateProgress(uint32_t y, uint32_t height);

  // [Dimensions checking]

  bool areDimensionsZero() const;
  bool areDimensionsTooLarge() const;

  // [Stream]

  virtual void attachStream(Stream& stream);
  virtual void detachStream();

  // [Protected virtuals]
protected:
  virtual void reset();

  // [Members]

  //! @brief Device provider.
  Provider* _provider;
  //! @brief Device type.
  uint32_t _deviceType;
  //! @brief Flags.
  uint32_t _flags;

  //! @brief Attached stream offset.
  uint64_t _attachedOffset;
  //! @brief Attached stream.
  Stream _stream;

  //! @brief Image or animation width.
  uint32_t _width;
  //! @brief Image or animation height.
  uint32_t _height;
  //! @brief Image or animation depth (bits per pixel).
  uint32_t _depth;
  //! @brief Image or animation planes.
  uint32_t _planes;

  //! @brief Actual frame.
  uint32_t _actualFrame;
  //! @brief Count of frames.
  uint32_t _framesCount;

  //! @brief Pixel format of target image.
  int _format;
  //! @brief Image format (see @c IMAGEIO_FILE_TYPE).
  int _imageType;

  //! @brief Palette if reading / writing 8 bit or less images.
  //!
  //! This is image palette that can be contained in loaded image,
  //! but it's not needed if image is directly loaded to different
  //! image format and decoder supports this.
  Palette _palette;
  //! @brief Comment.
  ByteArray _comment;

  //! @brief Progress, 0 to 100 [percent]
  float _progress;

private:
  friend struct IcoDecoderDevice;
};

// ============================================================================
// [Fog::ImageIO::DecoderDevice]
// ============================================================================

//! @brief Image IO decoder device.
struct FOG_API DecoderDevice : public BaseDevice
{
  FOG_DECLARE_OBJECT(DecoderDevice, BaseDevice)

  // [Construction / Descruction]

  DecoderDevice(Provider* provider);
  virtual ~DecoderDevice();

  // [Members access]

  FOG_INLINE bool isHeaderDone() const { return _headerDone; }
  FOG_INLINE bool isReaderDone() const { return _readerDone; }
  FOG_INLINE uint32_t getHeaderResult() const { return _headerResult; }
  FOG_INLINE uint32_t getReaderResult() const { return _readerResult; }

  // [Virtuals]

  virtual err_t readHeader() = 0;
  virtual err_t readImage(Image& image) = 0;

  // [Protected virtuals]
protected:
  virtual void reset();

  // [Members]

  //! @brief @c true if header was read.
  uint32_t _headerDone : 1;
  //! @brief @c true if image was read.
  uint32_t _readerDone : 1;
  //! @brief Header decoder result code (returned by @c readHeader()).
  uint32_t _headerResult;
  //! @brief Image decoder result code (returned by @c readImage()).
  uint32_t _readerResult;
};

// ============================================================================
// [Fog::ImageIO::EncoderDevice]
// ============================================================================

//! @brief Image IO encoder device.
struct FOG_API EncoderDevice : public BaseDevice
{
  FOG_DECLARE_OBJECT(EncoderDevice, BaseDevice)

  // [Construction / Descruction]

  EncoderDevice(Provider* provider);
  virtual ~EncoderDevice();

  // [Members access]

  FOG_INLINE bool isHeaderDone() const { return _headerDone; }
  FOG_INLINE bool isWriterDone() const { return _writerDone; }

  FOG_INLINE void setComment(const ByteArray& comment) { _comment = comment; }

  // [Virtuals]

  virtual void detachStream();
  virtual err_t writeImage(const Image& image) = 0;

protected:
  virtual void reset();
  virtual void finalize();

  // [Members]

  uint32_t _headerDone : 1;
  uint32_t _writerDone : 1;
};

} // ImageIO namespace
} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_IMAGEIO_H
