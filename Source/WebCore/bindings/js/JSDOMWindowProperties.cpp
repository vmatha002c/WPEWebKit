/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

#include "config.h"
#include "JSDOMWindowProperties.h"

#include "Frame.h"
#include "HTMLDocument.h"
#include "JSDOMBinding.h"
#include "JSDOMWindowBase.h"
#include "JSElement.h"
#include "JSHTMLCollection.h"

namespace WebCore {

using namespace JSC;

const ClassInfo JSDOMWindowProperties::s_info = { "WindowProperties", &Base::s_info, 0, CREATE_METHOD_TABLE(JSDOMWindowProperties) };

static bool jsDOMWindowPropertiesGetOwnPropertySlotNamedItemGetter(JSDOMWindowProperties* thisObject, Frame& frame, ExecState* exec, PropertyName propertyName, PropertySlot& slot)
{
    // Check for child frames by name before built-in properties to match Mozilla. This does
    // not match IE, but some sites end up naming frames things that conflict with window
    // properties that are in Moz but not IE. Since we have some of these, we have to do it
    // the Moz way.
    if (auto* scopedChild = frame.tree().scopedChild(propertyNameToAtomicString(propertyName))) {
        slot.setValue(thisObject, ReadOnly | DontDelete | DontEnum, toJS(exec, scopedChild->document()->domWindow()));
        return true;
    }

    // FIXME: Search the whole frame hierarchy somewhere around here.
    // We need to test the correct priority order.

    // Allow shortcuts like 'Image1' instead of document.images.Image1
    Document* document = frame.document();
    if (is<HTMLDocument>(*document)) {
        auto& htmlDocument = downcast<HTMLDocument>(*document);
        auto* atomicPropertyName = propertyName.publicName();
        if (atomicPropertyName && htmlDocument.hasWindowNamedItem(*atomicPropertyName)) {
            JSValue namedItem;
            if (UNLIKELY(htmlDocument.windowNamedItemContainsMultipleElements(*atomicPropertyName))) {
                Ref<HTMLCollection> collection = document->windowNamedItems(atomicPropertyName);
                ASSERT(collection->length() > 1);
                namedItem = toJS(exec, thisObject->globalObject(), collection);
            } else
                namedItem = toJS(exec, thisObject->globalObject(), htmlDocument.windowNamedItem(*atomicPropertyName));
            slot.setValue(thisObject, ReadOnly | DontDelete | DontEnum, namedItem);
            return true;
        }
    }

    return false;
}

bool JSDOMWindowProperties::getOwnPropertySlot(JSObject* object, ExecState* state, PropertyName propertyName, PropertySlot& slot)
{
    auto* thisObject = jsCast<JSDOMWindowProperties*>(object);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    if (Base::getOwnPropertySlot(thisObject, state, propertyName, slot))
        return true;
    JSValue proto = thisObject->getPrototypeDirect();
    if (proto.isObject() && jsCast<JSObject*>(proto)->hasProperty(state, propertyName))
        return false;

    auto& window = jsCast<JSDOMWindowBase*>(thisObject->globalObject())->wrapped();
    if (auto* frame = window.frame())
        return jsDOMWindowPropertiesGetOwnPropertySlotNamedItemGetter(thisObject, *frame, state, propertyName, slot);

    return false;
}

bool JSDOMWindowProperties::getOwnPropertySlotByIndex(JSObject* object, ExecState* state, unsigned index, PropertySlot& slot)
{
    return getOwnPropertySlot(object, state, Identifier::from(state, index), slot);
}

} // namespace WebCore
