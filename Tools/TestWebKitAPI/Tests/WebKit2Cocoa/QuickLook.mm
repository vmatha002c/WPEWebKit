/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "config.h"

#if PLATFORM(IOS)

#import "PlatformUtilities.h"
#import <WebKit/WKNavigationDelegatePrivate.h>
#import <WebKit/WKWebView.h>
#import <wtf/RetainPtr.h>

static bool isDone;
static bool didStartQuickLookLoad;
static bool didFinishQuickLookLoad;

@interface QuickLookNavigationDelegate : NSObject <WKNavigationDelegatePrivate>
@end

@implementation QuickLookNavigationDelegate

- (void)_webView:(WKWebView *)webView didStartLoadForQuickLookDocumentInMainFrameWithFileName:(NSString *)fileName uti:(NSString *)uti
{
    EXPECT_WK_STREQ(fileName, @"pages.pages");
    EXPECT_WK_STREQ(uti, @"com.apple.iwork.pages.sffpages");
    didStartQuickLookLoad = true;
}

- (void)_webView:(WKWebView *)webView didFinishLoadForQuickLookDocumentInMainFrame:(NSData *)documentData
{
    EXPECT_EQ(documentData.length, 274143U);
    didFinishQuickLookLoad = true;
}

- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation
{
    isDone = true;
}

@end

static void runTest(Class navigationDelegateClass)
{
    auto webView = adoptNS([[WKWebView alloc] init]);
    auto navigationDelegate = adoptNS([[navigationDelegateClass alloc] init]);
    [webView setNavigationDelegate:navigationDelegate.get()];
    [webView loadRequest:[NSURLRequest requestWithURL:[[NSBundle mainBundle] URLForResource:@"pages" withExtension:@"pages" subdirectory:@"TestWebKitAPI.resources"]]];

    isDone = false;
    TestWebKitAPI::Util::run(&isDone);
}

TEST(QuickLook, NavigationDelegate)
{
    runTest([QuickLookNavigationDelegate class]);
    EXPECT_TRUE(didStartQuickLookLoad);
    EXPECT_TRUE(didFinishQuickLookLoad);
}

@interface QuickLookDecidePolicyDelegate : NSObject <WKNavigationDelegate>
@end

@implementation QuickLookDecidePolicyDelegate

- (void)webView:(WKWebView *)webView decidePolicyForNavigationResponse:(WKNavigationResponse *)navigationResponse decisionHandler:(void (^)(WKNavigationResponsePolicy))decisionHandler
{
    decisionHandler(WKNavigationResponsePolicyCancel);
}

- (void)webView:(WKWebView *)webView didFailProvisionalNavigation:(WKNavigation *)navigation withError:(NSError *)error
{
    isDone = true;
}

@end

TEST(QuickLook, CancelNavigationAfterResponse)
{
    runTest([QuickLookDecidePolicyDelegate class]);
}

#endif // PLATFORM(IOS)
