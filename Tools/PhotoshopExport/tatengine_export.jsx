/*
 *  tatengine_export.jsx
 *  TatEngine
 *  based on work of Tomek Cejner, Damien van Holten, Jens Bache-wiig
 *
 *  Created by Dmitrii Ivanov on 08/15/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

// version 0.06, from 11.4.12
// contacts : jimon@tatemgames.com

#target photoshop

function main()
{
	if(!documents.length)
		return;

	var originalRulerUnits = app.preferences.rulerUnits;
	var originalTypeUnits = app.preferences.typeUnits;

	app.preferences.rulerUnits = Units.PIXELS;
	app.preferences.typeUnits = TypeUnits.PIXELS;

	// alert(app.version);

	loadXMPLibrary();

	var doc = activeDocument;
	var oldPath = activeDocument.path;

	var outFolder = new Folder(oldPath + "/exported");
	if(!outFolder.exists)
		outFolder.create();

	var jsonFile = new File(oldPath + "/exported/export.json");
	jsonFile.encoding = "UTF8";
	jsonFile.open("w", "TEXT", "");
	jsonFile.write("{\n");
	jsonFile.write("\t\"name\" : \"root\",\n");
	jsonFile.write("\t\"type\" : \"group\",\n");
	jsonFile.write("\t\"width\" : " + Math.floor(doc.width.as("px")) + ",\n");
	jsonFile.write("\t\"height\" : " + Math.floor(doc.height.as("px")) + ",\n");

	jsonFile.write("\t\"childs\" :\n");
	jsonFile.write("\t[\n");

	var totalIndex = scanLayerSets(doc, 0, 0);

	jsonFile.write("\t],\n");
	jsonFile.write("\t\"totalIndexes\" : " + totalIndex + "\n");
	jsonFile.write("}\n");
	jsonFile.close();

	app.preferences.rulerUnits = originalRulerUnits;
	app.preferences.typeUnits = originalTypeUnits;

	function tabs(lvl)
	{
		var str = "";
		for(var i = 0; i < lvl + 2; ++i)
			str += "\t";
		return str;
	}

	function escapeText(txt)
	{
		txt = txt.replace("\\", "\\\\");
		txt = txt.replace("\"", "\\\"");
		txt = txt.replace("\b", "\\b");
		txt = txt.replace("\f", "\\f");
		txt = txt.replace("\n", "\\n");
		txt = txt.replace("\r", "\\r");
		txt = txt.replace("\t", "\\t");
		return txt;
	}

	function scanLayerSets(el, index, lvl)
	{
		for(var i = 0; i < el.layers.length; i++)
		{
			var layer = el.layers[i];
			var layerName = escapeText(layer.name);

			if(layerName.charAt(0) == "#")
				continue;

			var opacity = layer.opacity / 100.0 * 255.0;

			jsonFile.write(tabs(lvl) + "{\n");
			jsonFile.write(tabs(lvl + 1) + "\"name\" : \"" + layerName + "\",\n");

			var metaData = null;

			try
			{
				var xmp = new XMPMeta(layer.xmpMetadata.rawData);
				var temp = xmp.getProperty(XMPConst.NS_EXIF, "tatengineMetaData").toString();
				jsonFile.write(tabs(lvl + 1) + "\"metaData\" : \"" + temp + "\",\n");

				metaData = eval("(" + Base64.decode(temp) + ")");
			}
			catch(e)
			{
			}

			jsonFile.write(tabs(lvl + 1) + "\"visible\" : " + layer.visible + ",\n");
			jsonFile.write(tabs(lvl + 1) + "\"opacity\" : " + opacity + ",\n");

			if(layer.blendMode == BlendMode.COLORBLEND) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"colorBlend\",\n");
			else if(layer.blendMode == BlendMode.COLORBURN) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"colorBurn\",\n");
			else if(layer.blendMode == BlendMode.COLORDODGE) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"colorDodge\",\n");
			else if(layer.blendMode == BlendMode.DARKEN) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"darken\",\n");
			else if(layer.blendMode == BlendMode.DIFFERENCE) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"difference\",\n");
			else if(layer.blendMode == BlendMode.DISSOLVE) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"dissolve\",\n");
			else if(layer.blendMode == BlendMode.DIVIDE) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"divide\",\n");
			else if(layer.blendMode == BlendMode.EXCLUSION) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"exclusion\",\n");
			else if(layer.blendMode == BlendMode.HARDLIGHT) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"hardLight\",\n");
			else if(layer.blendMode == BlendMode.HARDMIX) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"hardMix\",\n");
			else if(layer.blendMode == BlendMode.HUE) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"hue\",\n");
			else if(layer.blendMode == BlendMode.LIGHTEN) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"lighten\",\n");
			else if(layer.blendMode == BlendMode.LINEARBURN) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"linearBurn\",\n");
			else if(layer.blendMode == BlendMode.LINEARDODGE) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"linearDodge\",\n");
			else if(layer.blendMode == BlendMode.LINEARLIGHT) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"linearLight\",\n");
			else if(layer.blendMode == BlendMode.LUMINOSITY) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"luminosity\",\n");
			else if(layer.blendMode == BlendMode.MULTIPLY) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"multiply\",\n");
			else if(layer.blendMode == BlendMode.NORMAL) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"normal\",\n");
			else if(layer.blendMode == BlendMode.OVERLAY) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"overlay\",\n");
			else if(layer.blendMode == BlendMode.PASSTHROUGH) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"passthrough\",\n");
			else if(layer.blendMode == BlendMode.PINLIGHT) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"pinLight\",\n");
			else if(layer.blendMode == BlendMode.SATURATION) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"saturation\",\n");
			else if(layer.blendMode == BlendMode.SCREEN) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"screen\",\n");
			else if(layer.blendMode == BlendMode.SOFTLIGHT) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"softLight\",\n");
			else if(layer.blendMode == BlendMode.SUBTRACT) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"subtract\",\n");
			else if(layer.blendMode == BlendMode.VIVIDLIGHT) jsonFile.write(tabs(lvl + 1) + "\"blendMode\" : \"vividLight\",\n");

			if(layer.typename == "LayerSet")
			{
				jsonFile.write(tabs(lvl + 1) + "\"type\" : \"group\",\n");
				jsonFile.write(tabs(lvl + 1) + "\"childs\" :\n");
				jsonFile.write(tabs(lvl + 1) + "[\n");

				index = scanLayerSets(layer, index, lvl + 2);

				jsonFile.write(tabs(lvl + 1) + "]\n");
			}
			else
			{
				jsonFile.write(tabs(lvl + 1) + "\"nodeIndex\" : " + index + ",\n");

				index++;

				var xoffset = layer.bounds[0].as("px");
				var yoffset = layer.bounds[1].as("px");

				if(xoffset < 0.0) xoffset = 0;
				if(yoffset < 0.0) yoffset = 0;

				var xoffset2 = Math.min(layer.bounds[2].as("px"), doc.width.as("px"));
				var yoffset2 = Math.min(layer.bounds[3].as("px"), doc.height.as("px"));

				var imageScale = 1.0;

				if(metaData != null)
				{
					for(var j = 0; j < metaData.length; j++)
					{
						if(metaData[j].type == "cutToWidth")
						{
							xoffset2 = xoffset + metaData[j].width;
						}
						else if(metaData[j].type == "scale")
						{
							imageScale = metaData[j].scale;
						}
					}
				}

				if(layer.kind == LayerKind.TEXT)
				{
					var textItem = layer.textItem;

					var fontSize = textItem.size.as("px");

					if(app.version === "13.0.0")
						fontSize = (fontSize * 60.0 / 35.4); // CS6, WTF ?!

					fontSize = Math.round(fontSize);

					//yoffset -= fontSize / 4; // hack from qml exporter
					//yoffset -= 2; // hack

					var textContents = escapeText(textItem.contents);

					jsonFile.write(tabs(lvl + 1) + "\"type\" : \"text\",\n");
					jsonFile.write(tabs(lvl + 1) + "\"text\" : \"" + textContents + "\",\n");

					jsonFile.write(tabs(lvl + 1) + "\"fontSize\" : " + fontSize + ",\n");
					jsonFile.write(tabs(lvl + 1) + "\"fontFamily\" : \"" + textItem.font + "\",\n");

					if(textItem.font.indexOf("Bold") != -1)
						jsonFile.write(tabs(lvl + 1) + "\"fontBold\" : \"true\",\n");
					else
						jsonFile.write(tabs(lvl + 1) + "\"fontBold\" : \"false\",\n");

					if(textItem.font.indexOf("Italic") != -1)
						jsonFile.write(tabs(lvl + 1) + "\"fontItalic\" : \"true\",\n");
					else
						jsonFile.write(tabs(lvl + 1) + "\"fontItalic\" : \"false\",\n");

					jsonFile.write(tabs(lvl + 1) + "\"colorRed\" : " + Math.floor(textItem.color.rgb.red) + ",\n");
					jsonFile.write(tabs(lvl + 1) + "\"colorGreen\" : " + Math.floor(textItem.color.rgb.green) + ",\n");
					jsonFile.write(tabs(lvl + 1) + "\"colorBlue\" : " + Math.floor(textItem.color.rgb.blue) + ",\n");

					//textItem.position - origin of text
					if((textItem.justification == Justification.CENTER) || (textItem.justification == Justification.CENTERJUSTIFIED) || (textItem.justification == Justification.FULLYJUSTIFIED))
						jsonFile.write(tabs(lvl + 1) + "\"halign\" : \"center\",\n");
					else if((textItem.justification == Justification.LEFT) || (textItem.justification == Justification.LEFTJUSTIFIED))
						jsonFile.write(tabs(lvl + 1) + "\"halign\" : \"left\",\n");
					else if((textItem.justification == Justification.RIGHT) || (textItem.justification == Justification.RIGHTJUSTIFIED))
						jsonFile.write(tabs(lvl + 1) + "\"halign\" : \"right\",\n");

					try
					{
						if(!textItem.useAutoLeading)
							jsonFile.write(tabs(lvl + 1) + "\"leading\" : " + textItem.leading.as("px") + ",\n");
					}
					catch(e)
					{
					}

					try
					{
						jsonFile.write(tabs(lvl + 1) + "\"tracking\" : " + textItem.tracking + ",\n");
					}
					catch(e)
					{
					}
				}
				else
				{
					var fileName = layerName.replace(/[ :\/\\*\?\"\<\>\|#]/g, "_") + ".png";

					if(fileName.length > 150)
						fileName = fileName.substring(0, 150);

					saveLayer(layer, fileName, oldPath, false, xoffset, yoffset, xoffset2, yoffset2, imageScale);

					jsonFile.write(tabs(lvl + 1) + "\"type\" : \"image\",\n");
					jsonFile.write(tabs(lvl + 1) + "\"fileName\" : \"" + fileName + "\",\n");
				}

				jsonFile.write(tabs(lvl + 1) + "\"x\" : " + xoffset + ",\n");
				jsonFile.write(tabs(lvl + 1) + "\"y\" : " + yoffset + ",\n");
				jsonFile.write(tabs(lvl + 1) + "\"width\" : " + (xoffset2 - xoffset) + ",\n");
				jsonFile.write(tabs(lvl + 1) + "\"height\" : " + (yoffset2 - yoffset) + "\n");
			}

			if(i == el.layers.length - 1)
				jsonFile.write(tabs(lvl) + "}\n");
			else
				jsonFile.write(tabs(lvl) + "},\n");
		}

		return index;
	}

	function saveLayer(layer, lname, path, shouldMerge, xoffset, yoffset, xoffset2, yoffset2, imageScale)
	{
		var opacity = layer.opacity;
		layer.opacity = 100.0;

		activeDocument.activeLayer = layer;
		dupLayers();

		if(shouldMerge === true)
			activeDocument.mergeVisibleLayers();

		//activeDocument.trim(TrimType.TRANSPARENT, true, true, true, true);
		activeDocument.crop([xoffset, yoffset, xoffset2, yoffset2]);

		//if((imageScale < 1.0) || (imageScale > 1.0))
			activeDocument.resizeImage(activeDocument.width.as("px") * imageScale, null, null, ResampleMethod.BICUBIC);

		var saveFile = File(path + "/exported/" + lname);
		SavePNG(saveFile);
		app.activeDocument.close(SaveOptions.DONOTSAVECHANGES);

		layer.opacity = opacity;
	}

	function dupLayers()
	{
		var desc143 = new ActionDescriptor();
		var ref73 = new ActionReference();
		ref73.putClass(charIDToTypeID('Dcmn'));
		desc143.putReference(charIDToTypeID('null'), ref73);
		desc143.putString(charIDToTypeID('Nm  '), activeDocument.activeLayer.name);
		var ref74 = new ActionReference();
		ref74.putEnumerated(charIDToTypeID('Lyr '), charIDToTypeID('Ordn'), charIDToTypeID('Trgt'));
		desc143.putReference(charIDToTypeID('Usng'), ref74);
		executeAction(charIDToTypeID('Mk  '), desc143, DialogModes.NO);
	};

	function SavePNG(saveFile)
	{
		var pngOpts = new ExportOptionsSaveForWeb; 
		pngOpts.format = SaveDocumentType.PNG
		pngOpts.PNG8 = false; 
		pngOpts.transparency = true; 
		pngOpts.interlaced = false; 
		pngOpts.quality = 100;
		activeDocument.exportDocument(new File(saveFile),ExportType.SAVEFORWEB,pngOpts); 
	}

	function loadXMPLibrary()
	{
		if(!ExternalObject.AdobeXMPScript)
		{
			try
			{
				ExternalObject.AdobeXMPScript = new ExternalObject('lib:AdobeXMPScript');
			}
			catch(e)
			{
				alert("Can't load XMP Script Library");
				return false;
			}
		}
		return true;
	}

	function unloadXMPLibrary()
	{
		if(ExternalObject.AdobeXMPScript)
		{
			try
			{
				ExternalObject.AdobeXMPScript.unload();
				ExternalObject.AdobeXMPScript = undefined;
			}
			catch(e)
			{
				alert("Can't unload XMP Script Library");
			}
		}
	}
};

var Base64 = {

// private property
_keyStr : "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=",

// public method for encoding
encode : function (input) {
    var output = "";
    var chr1, chr2, chr3, enc1, enc2, enc3, enc4;
    var i = 0;

    input = Base64._utf8_encode(input);

    while (i < input.length) {

        chr1 = input.charCodeAt(i++);
        chr2 = input.charCodeAt(i++);
        chr3 = input.charCodeAt(i++);

        enc1 = chr1 >> 2;
        enc2 = ((chr1 & 3) << 4) | (chr2 >> 4);
        enc3 = ((chr2 & 15) << 2) | (chr3 >> 6);
        enc4 = chr3 & 63;

        if (isNaN(chr2)) {
            enc3 = enc4 = 64;
        } else if (isNaN(chr3)) {
            enc4 = 64;
        }

        output = output +
        this._keyStr.charAt(enc1) + this._keyStr.charAt(enc2) +
        this._keyStr.charAt(enc3) + this._keyStr.charAt(enc4);

    }

    return output;
},

// public method for decoding
decode : function (input) {
    var output = "";
    var chr1, chr2, chr3;
    var enc1, enc2, enc3, enc4;
    var i = 0;

    input = input.replace(/[^A-Za-z0-9\+\/\=]/g, "");

    while (i < input.length) {

        enc1 = this._keyStr.indexOf(input.charAt(i++));
        enc2 = this._keyStr.indexOf(input.charAt(i++));
        enc3 = this._keyStr.indexOf(input.charAt(i++));
        enc4 = this._keyStr.indexOf(input.charAt(i++));

        chr1 = (enc1 << 2) | (enc2 >> 4);
        chr2 = ((enc2 & 15) << 4) | (enc3 >> 2);
        chr3 = ((enc3 & 3) << 6) | enc4;

        output = output + String.fromCharCode(chr1);

        if (enc3 != 64) {
            output = output + String.fromCharCode(chr2);
        }
        if (enc4 != 64) {
            output = output + String.fromCharCode(chr3);
        }

    }

    output = Base64._utf8_decode(output);

    return output;

},

// private method for UTF-8 encoding
_utf8_encode : function (string) {
    string = string.replace(/\r\n/g,"\n");
    var utftext = "";

    for (var n = 0; n < string.length; n++) {

        var c = string.charCodeAt(n);

        if (c < 128) {
            utftext += String.fromCharCode(c);
        }
        else if((c > 127) && (c < 2048)) {
            utftext += String.fromCharCode((c >> 6) | 192);
            utftext += String.fromCharCode((c & 63) | 128);
        }
        else {
            utftext += String.fromCharCode((c >> 12) | 224);
            utftext += String.fromCharCode(((c >> 6) & 63) | 128);
            utftext += String.fromCharCode((c & 63) | 128);
        }

    }

    return utftext;
},

// private method for UTF-8 decoding
_utf8_decode : function (utftext) {
    var string = "";
    var i = 0;
    var c = c1 = c2 = 0;

    while ( i < utftext.length ) {

        c = utftext.charCodeAt(i);

        if (c < 128) {
            string += String.fromCharCode(c);
            i++;
        }
        else if((c > 191) && (c < 224)) {
            c2 = utftext.charCodeAt(i+1);
            string += String.fromCharCode(((c & 31) << 6) | (c2 & 63));
            i += 2;
        }
        else {
            c2 = utftext.charCodeAt(i+1);
            c3 = utftext.charCodeAt(i+2);
            string += String.fromCharCode(((c & 15) << 12) | ((c2 & 63) << 6) | (c3 & 63));
            i += 3;
        }

    }

    return string;
}

}

main();
