//
//  ColorUtils.hpp
//  RGB-ColorDifference
//
//  Created by Mohamed Shahawy on 4/22/16.

//  This project is under the MIT license
//
//  Copyright © 2016 Mohamed Shahawy
// 
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
//  and associated documentation files (the “Software”), to deal in the Software without restriction, 
//  including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
//  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
//  subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
//  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
//  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

//  Special thanks to GitHub @rerdavies for highlighting the copyright pitfall in this project.



#ifndef ColorUtils_hpp
#define ColorUtils_hpp

class ColorUtils
{
public:     // General-use structs
    struct xyzColor
    {
        float x, y, z;
        
        xyzColor(){}
        xyzColor(float x, float y, float z)
        {
            this->x = x; this->y = y; this->z = z;
        }
        xyzColor(const xyzColor& c)
        {
            x = c.x; y = c.y; z = c.z;
        }
        xyzColor& operator=(const xyzColor& c)
        {
            x = c.x; y = c.y; z = c.z;
            return *this;
        }
    };
    struct CIELABColorSpace
    {
        float l, a, b;
        
        CIELABColorSpace(){}
        CIELABColorSpace(float l, float a, float b)
        {
            this->l = l; this->a = a; this->b = b;
        }
        CIELABColorSpace(const CIELABColorSpace& c)
        {
            l = c.l; a = c.a; b = c.b;
        }
        CIELABColorSpace& operator=(const CIELABColorSpace& c)
        {
            l = c.l; a = c.a; b = c.b;
            return *this;
        }
    };
    struct rgbColor
    {
        unsigned int r, g, b;
        float rF, gF, bF;
        
        rgbColor(){}
        rgbColor(unsigned int r, unsigned int g, unsigned int b)
        {
            this->r = r % 256; this->g = g % 256; this->b = b % 256;
            initFloats();
        }
        rgbColor(float r, float g, float b)
        {
            this->rF = r; this->gF = g; this->bF = b;
            initInts();
        }
        rgbColor(const rgbColor& c)
        {
            r = c.r; g = c.g; b = c.b;
            initFloats();
        }
//        rgbColor(const Color4F& c)        // Cocos2D-X Color4F constructor
//        {
//            Color4B x(c);
//            r = x.r; g = x.g; b = x.b;
//        }
        rgbColor& operator=(const rgbColor& c)
        {
            r = c.r; g = c.g; b = c.b;
            initFloats();
            return *this;
        }
        
    private:
        void initFloats()
        {
            rF = r / 255.0; gF = g / 255.0; bF = b / 255.0;
        }
        void initInts()
        {
            r = rF * 255.0; g = gF * 255.0; b = bF * 255.0;
        }
    };
    
    
    // Functions
    static float getColorDeltaE(rgbColor c1, rgbColor c2);
    static xyzColor rgbToXyz(rgbColor c);
    static CIELABColorSpace xyzToCIELAB(xyzColor c);
};

#endif /* ColorUtils_hpp */
