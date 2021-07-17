package com.wind.hiddenapi.bypass;

/**
 * @author WindySha (https://github.com/WindySha)
 */
public class HiddenApiBypass {
    static {
        System.loadLibrary("bypassRestriction");
    }

    public static void startBypass() {
        setHiddenApiExemptions(new String[]{"L"});
    }

    /**
     * Sets the list of exemptions from hidden API access enforcement.
     *
     * @param signaturePrefixes A list of signature prefixes. Each item in the list is a prefix match on the type
     *                          signature of a blacklisted API. All matching APIs are treated as if they were on
     *                          the whitelist: access permitted, and no logging..
     */
    public static native void setHiddenApiExemptions(String[] signaturePrefixes);
}