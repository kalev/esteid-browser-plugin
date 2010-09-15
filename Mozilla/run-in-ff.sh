#!/bin/sh

export MOZ_NO_REMOTE=1

trunkdir=$(cd "`dirname $0`/.."; pwd)
tstdir=`cd "$trunkdir/testpage"; pwd`
[ "$1" ] && xpi="$1" || xpi=esteid-*.xpi
tmpdir=`mktemp -d /tmp/ffprofile.XXXXXXXX`
trap 'rm -rf "$tmpdir"' INT
xdir="$tmpdir/extensions/{aa84ce40-4253-11da-8cd6-0800200c9a66}"
pdir="$tmpdir/plugins"
mkdir -p "$xdir"
mkdir -p "$pdir"
unzip -d "$xdir" "$xpi" > /dev/null
[ "$2" ] && cp "$2"/* "$pdir"
cat >> "$tmpdir"/bookmarks.html <<E_O_F
<!DOCTYPE NETSCAPE-Bookmark-file-1>
<META HTTP-EQUIV="Content-Type" CONTENT="text/html; charset=UTF-8">
<TITLE>Bookmarks</TITLE>
<H1>Bookmarks menu</H1>

<DL><p>
    <DT><H3 PERSONAL_TOOLBAR_FOLDER="true">Toolbar folder</H3>
    <DL><p>
        <DT><A HREF="file://$bdir">Build dir</A>
        <DT><A HREF="file://$tstdir/test.html">test.html</A>
        <DT><A HREF="https://id.smartlink.ee/plugin_tests/test.html">plugin_tests</A>
    </DL><p>
</DL><p>
E_O_F

# Enable extension development preferences
# https://developer.mozilla.org/en/Setting_up_extension_development_environment

cat >> "$tmpdir"/prefs.js <<E_O_F
user_pref("javascript.options.showInConsole", true);
user_pref("nglayout.debug.disable_xul_cache", true);
user_pref("browser.dom.window.dump.enabled", true);
user_pref("javascript.options.strict", true);
user_pref("extensions.logging.enabled", true);
user_pref("nglayout.debug.disable_xul_fastload", true);
E_O_F

# Add JavaScript debugger extension if ENABLE_VENKMAN is set
vurl="https://addons.mozilla.org/en-US/firefox/downloads/latest/216/addon-216-latest.xpi"
vdir="$tmpdir/extensions/{f13b157f-b174-47e7-a34d-4815ddfdfeb8}"
vxpi="/tmp/venkman.xpi"
if [ "$ENABLE_VENKMAN" ]; then
    [ -f "$vxpi" ] || wget -O "$vxpi" "$vurl"
    mkdir -p $vdir
    unzip -d "$vdir" "$vxpi" > /dev/null
fi

firefox -profile "$tmpdir" "about:blank"

rm -rf "$tmpdir"
