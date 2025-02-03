//TODO: have a functionality to deal with individual keys and function pointer for user program


#include <objc/runtime.h>
#include <objc/message.h>
#include <ApplicationServices/ApplicationServices.h>
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#include "platform.h"

#define objc_args_int(...) ((int(*)(__VA_ARGS__))objc_msgSend)_send_obj_msg
#define objc_args_void(...) ((void(*)(__VA_ARGS__))objc_msgSend)_send_obj_msg
#define objc_args_id(...) ((id(*)(__VA_ARGS__))objc_msgSend)_send_obj_msg
#define _send_obj_msg(_Id, _Sel, ...) ((id)_Id, sel_registerName(_Sel), ##__VA_ARGS__)
#define cust_alloc(type) (((id (*)(id, SEL))objc_msgSend)((id)type, sel_registerName("alloc")))

static id _app, _window, _gl_context, _event_loop_mode, _event_resp_time, _delegate;

enum:uint32_t {
    NSWindowStyleMaskBorderless = 0,
    NSWindowStyleMaskTitled = 1 << 0,
    NSWindowStyleMaskClosable = 1 << 1,
    NSWindowStyleMaskMiniaturizable = 1 << 2,
    NSWindowStyleMaskResizable	= 1 << 3,
    NSWindowStyleMaskUnifiedTitleAndToolbar = 1 << 12,
    NSWindowStyleMaskFullScreen = 1 << 14,
    NSWindowStyleMaskFullSizeContentView = 1 << 15,
    NSWindowStyleMaskUtilityWindow = 1 << 4,
    NSWindowStyleMaskDocModalWindow = 1 << 6,
    NSWindowStyleMaskNonactivatingPanel = 1 << 7,
    NSWindowStyleMaskHUDWindow = 1 << 13
};

enum:uint32_t {
    NSBackingStoreRetained = 0,
    NSBackingStoreNonretained = 1,
    NSBackingStoreBuffered = 2
};

enum:uint32_t {
    NSEventTypeLeftMouseDown             = 1,
    NSEventTypeLeftMouseUp               = 2,
    NSEventTypeRightMouseDown            = 3,
    NSEventTypeRightMouseUp              = 4,
    NSEventTypeMouseMoved                = 5,
    NSEventTypeLeftMouseDragged          = 6,
    NSEventTypeRightMouseDragged         = 7,
    NSEventTypeMouseEntered              = 8,
    NSEventTypeMouseExited               = 9,
    NSEventTypeKeyDown                   = 10,
    NSEventTypeKeyUp                     = 11,
    NSEventTypeFlagsChanged              = 12,
    NSEventTypeAppKitDefined             = 13,
    NSEventTypeSystemDefined             = 14,
    NSEventTypeApplicationDefined        = 15,
    NSEventTypePeriodic                  = 16,
    NSEventTypeCursorUpdate              = 17,
    NSEventTypeScrollWheel               = 22,
    NSEventTypeTabletPoint               = 23,
    NSEventTypeTabletProximity           = 24,
    NSEventTypeOtherMouseDown            = 25,
    NSEventTypeOtherMouseUp              = 26,
    NSEventTypeOtherMouseDragged         = 27,
 };

enum:uint8_t {
    NSApplicationActivationPolicyRegular,
    NSApplicationActivationPolicyAccessory,
    NSApplicationActivationPolicyProhibited
};

enum {
    NSOpenGLPFAAllRenderers           =   1,
    NSOpenGLPFATripleBuffer           =   3,
    NSOpenGLPFADoubleBuffer           =   5,
    NSOpenGLPFAAuxBuffers             =   7,
    NSOpenGLPFAColorSize              =   8,
    NSOpenGLPFAAlphaSize              =  11,
    NSOpenGLPFADepthSize              =  12,
    NSOpenGLPFAStencilSize            =  13,
    NSOpenGLPFAAccumSize              =  14,
    NSOpenGLPFAMinimumPolicy          =  51,
    NSOpenGLPFAMaximumPolicy          =  52,
    NSOpenGLPFASampleBuffers          =  55,
    NSOpenGLPFASamples                =  56,
    NSOpenGLPFAAuxDepthStencil        =  57,
    NSOpenGLPFAColorFloat             =  58,
    NSOpenGLPFAMultisample            =  59,
    NSOpenGLPFASupersample            =  60,
    NSOpenGLPFASampleAlpha            =  61,
    NSOpenGLPFARendererID             =  70,
    NSOpenGLPFANoRecovery             =  72,
    NSOpenGLPFAAccelerated            =  73,
    NSOpenGLPFAClosestPolicy          =  74,
    NSOpenGLPFABackingStore           =  76,
    NSOpenGLPFAScreenMask             =  84,
    NSOpenGLPFAAllowOfflineRenderers  =  96,
    NSOpenGLPFAAcceleratedCompute     =  97,
    NSOpenGLPFAOpenGLProfile          =  99,
    NSOpenGLPFAVirtualScreenCount     = 128,
    
    NSOpenGLPFAStereo                 =   6,
    NSOpenGLPFAOffScreen              =  53,
    NSOpenGLPFAFullScreen             =  54,
    NSOpenGLPFASingleRenderer         =  71,
    NSOpenGLPFARobust                 =  75,
    NSOpenGLPFAMPSafe                 =  78,
    NSOpenGLPFAWindow                 =  80,
    NSOpenGLPFAMultiScreen            =  81,
    NSOpenGLPFACompliant              =  83,
    NSOpenGLPFAPixelBuffer            =  90,
    NSOpenGLPFARemotePixelBuffer      =  91,
};

/* NSOpenGLPFAOpenGLProfile values */
enum {
    NSOpenGLProfileVersionLegacy 	= 0x1000,
    NSOpenGLProfileVersion3_2Core	= 0x3200,
    NSOpenGLProfileVersion4_1Core	= 0x4100 
};

enum:uint32_t {
    NSOpenGLContextParameterSwapInterval           = 222, /* 1 param.  0 -> Don't sync, 1 -> Sync to vertical retrace     */
    NSOpenGLContextParameterSurfaceOrder           = 235, /* 1 param.  1 -> Above Window (default), -1 -> Below Window    */
    NSOpenGLContextParameterSurfaceOpacity         = 236, /* 1 param.  1-> Surface is opaque (default), 0 -> non-opaque   */
    NSOpenGLContextParameterSurfaceBackingSize     = 304, /* 2 params.  Width/height of surface backing size              */
    NSOpenGLContextParameterReclaimResources       = 308, /* 0 params.                                                    */
    NSOpenGLContextParameterCurrentRendererID      = 309, /* 1 param.   Retrieves the current renderer ID                 */
    NSOpenGLContextParameterGPUVertexProcessing    = 310, /* 1 param.   Currently processing vertices with GPU (get)      */
    NSOpenGLContextParameterGPUFragmentProcessing  = 311, /* 1 param.   Currently processing fragments with GPU (get)     */
    NSOpenGLContextParameterHasDrawable            = 314, /* 1 param.   Boolean returned if drawable is attached          */
    NSOpenGLContextParameterMPSwapsInFlight        = 315, /* 1 param.   Max number of swaps queued by the MP GL engine    */
    
    NSOpenGLContextParameterSwapRectangle  			= 200, /* 4 params.  Set or get the swap rectangle {x, y, w, h} */
    NSOpenGLContextParameterSwapRectangleEnable  	= 201, /* Enable or disable the swap rectangle */
    NSOpenGLContextParameterRasterizationEnable  	= 221, /* Enable or disable all rasterization */
    NSOpenGLContextParameterStateValidation  		= 301, /* Validate state for multi-screen functionality */
    NSOpenGLContextParameterSurfaceSurfaceVolatile  = 306, /* 1 param.   Surface volatile state */
 };

bool windowShouldClose(id self) {
	fprintf(stderr, "window should close\n");
	return true;
}

static void SetupGL(void) {
	Class gl_pixel_format_class = objc_getClass("NSOpenGLPixelFormat");
	Class gl_context_class		= objc_getClass("NSOpenGLContext");
	const uint32_t attribs[] = {
		NSOpenGLPFAOpenGLProfile,
		NSOpenGLProfileVersion4_1Core,
		NSOpenGLPFAColorSize, 24,
		NSOpenGLPFADepthSize, 24,
		NSOpenGLPFAAlphaSize, 8,
		NSOpenGLPFADoubleBuffer,
		0    // array termination
	};

	id pixel_format = objc_args_id(id, SEL, const uint32_t*)
					(cust_alloc(gl_pixel_format_class), 
					 "initWithAttributes:", attribs);
	_gl_context = objc_args_id(id, SEL, id, id)
								(cust_alloc(gl_context_class), 
								 "initWithFormat:shareContext:", 
								 pixel_format, nullptr);
	objc_args_void(id, SEL)(pixel_format, "dealloc");
	id context_view = objc_args_id(id, SEL)(_window, "contentView");
	objc_args_void(id, SEL, id)(_gl_context, "setView:", context_view);
	objc_args_void(id, SEL)(_gl_context, "makeCurrentContext");
}

void PlatformPollEvents(void) {
	const char* flag = "nextEventMatchingMask:untilDate:inMode:dequeue:";
	id event = objc_args_id(id, SEL, size_t, void*, id, bool)
						(_app, flag, SIZE_T_MAX, _event_resp_time, _event_loop_mode, true);
	if (event != nullptr) {
		int event_type = objc_args_int(id, SEL)(event, "type");
		if (event_type == NSEventTypeKeyDown) {
			fprintf(stderr, "key pressed\n");
			objc_args_void(id, SEL, id)(_window, "performClose:", nullptr);
		}
	}
	objc_args_void(id, SEL, id)(_app, "sendEvent:", event);
	objc_args_void(id, SEL)(_app, "updateWindows");
}

inline bool PlatformShouldWindowClose(void) {
	return !objc_args_int(id, SEL)(_window, "isVisible");
}

void PlatformCloseWindow(void) {
	objc_args_void(id, SEL)(_event_loop_mode, "dealloc");
	objc_args_void(id, SEL)(_window, "dealloc");
	objc_args_void(id, SEL)(_app, "dealloc");
	objc_args_void(id, SEL)(_delegate, "dealloc");
}

void PlatformCreateWindow(const uint32_t width, const uint32_t height, const char* title) {
	assert(_app == nullptr); // check for duplicate app
	_event_loop_mode = objc_args_id(id, SEL, const char*)(objc_getClass("NSString"), "stringWithUTF8String:", "NSEventTrackingRunLoopMode");
	const float target_fps = 1/60.0f;
	_event_resp_time = objc_args_id(id, SEL, double)(objc_getClass("NSDate"), "dateWithTimeIntervalSinceNow:", target_fps); //100ms
	_app = objc_args_id(id, SEL)(objc_getClass("NSApplication"), "sharedApplication");
    objc_args_void(id, SEL, int)(_app, "setActivationPolicy:", NSApplicationActivationPolicyRegular);
	_window = objc_args_id(id, SEL, CGRect, uint32_t, uint32_t, BOOL)(cust_alloc(objc_getClass("NSWindow")), 
	   																"initWithContentRect:styleMask:backing:defer:", 
																	CGRectMake(0, 0, width, height),
																	(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable),
																	NSBackingStoreBuffered,
																	false);
	Class window_delegate_class = objc_allocateClassPair(objc_getClass("NSObject"), "WindowDelegate", 0);
	objc_registerClassPair(window_delegate_class);
	class_addMethod(window_delegate_class, sel_registerName("windowShouldClose:"), (IMP)windowShouldClose, "c@");

	_delegate = objc_args_id(id, SEL)(cust_alloc(window_delegate_class), "init");
	objc_args_id(id, SEL, id)(_window, "setDelegate:", _delegate);

	PlatformSetWindowTitle(title);

	objc_args_void(id, SEL, id)(_window, "makeKeyAndOrderFront:", nullptr);
	objc_args_void(id, SEL)(_app, "finishLaunching");
	SetupGL();
}

inline void PlatformSwapBuffers(void) {
	objc_args_void(id, SEL)(_gl_context, "flushBuffer");
}

void PlatformSetWindowTitle(const char* title) {
	id window_title = objc_args_id(id, SEL, const char*)(objc_getClass("NSString"), 
														"stringWithUTF8String:", title);
	objc_args_void(id, SEL, id)(_window, "setTitle:", window_title);
	objc_args_void(id, SEL)(window_title, "dealloc");
}

inline void PlatformGetCurrentTime(_platform_time* t) {
	clock_gettime(CLOCK_MONOTONIC_RAW_APPROX, (struct timespec *)t);
}

inline void PlatformSetSwapInterval(uint8_t interval) {
	objc_args_void(id, SEL, uint8_t*, uint32_t)
		(_gl_context, "setValues:forParameter:", &interval, NSOpenGLContextParameterSwapInterval);
}
