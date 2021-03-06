//
// Copyright (c) 2017 The Khronos Group Inc.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#define _CRT_SECURE_NO_WARNINGS
#include "harness.h"

Texture2DSize texture2DSizes[] =
{
    {
        4, // Width
        4, // Height
        1, // MipLevels
        1, // ArraySize
        1, // SubResourceCount
        {  // SubResources
            {0, 0}, // MipLevel, ArraySlice
            {0, 0}, // MipLevel, ArraySlice
            {0, 0}, // MipLevel, ArraySlice
            {0, 0}, // MipLevel, ArraySlice
        },
        0, // MiscFlags
    },
    {
        15, // Width
        37, // Height
        2, // MipLevels
        1, // ArraySize
        2, // SubResourceCount
        {  // SubResources
            {0, 0}, // MipLevel, ArraySlice
            {1, 0}, // MipLevel, ArraySlice
            {0, 0}, // MipLevel, ArraySlice
            {0, 0}, // MipLevel, ArraySlice
        },
        0, // MiscFlags
    },
    {
        65, // Width
        17, // Height
        1, // MipLevels
        1, // ArraySize
        1, // SubResourceCount
        {  // SubResources
            {0, 0}, // MipLevel, ArraySlice
            {0, 0}, // MipLevel, ArraySlice
            {0, 0}, // MipLevel, ArraySlice
            {0, 0}, // MipLevel, ArraySlice
        },
        D3D10_RESOURCE_MISC_SHARED, // MiscFlags
    },

    {
        127, // Width
        125, // Height
        4, // MipLevels
        1, // ArraySize
        4, // SubResourceCount
        {  // SubResources
            {3, 0}, // MipLevel, ArraySlice
            {2, 0}, // MipLevel, ArraySlice
            {1, 0}, // MipLevel, ArraySlice
            {0, 0}, // MipLevel, ArraySlice
        },
        0, // MiscFlags
    },
    {
        128, // Width
        128, // Height
        4, // MipLevels
        6, // ArraySize
        4, // SubResourceCount
        {  // SubResources
            {0, 1}, // MipLevel, ArraySlice
            {1, 0}, // MipLevel, ArraySlice
            {0, 2}, // MipLevel, ArraySlice
            {3, 5}, // MipLevel, ArraySlice
        },
        0, // MiscFlags
    },
    {
        256, // Width
        256, // Height
        0, // MipLevels
        256, // ArraySize
        4, // SubResourceCount
        {  // SubResources
            {0,   0}, // MipLevel, ArraySlice
            {1, 255}, // MipLevel, ArraySlice
            {2, 127}, // MipLevel, ArraySlice
            {3, 128}, // MipLevel, ArraySlice
        },
        0, // MiscFlags
    },
    {
        258, // Width
        511, // Height
        1, // MipLevels
        1, // ArraySize
        1, // SubResourceCount
        {  // SubResources
            {0, 0}, // MipLevel, ArraySlice
            {0, 0}, // MipLevel, ArraySlice
            {0, 0}, // MipLevel, ArraySlice
            {0, 0}, // MipLevel, ArraySlice
        },
        0, // MiscFlags
    },
    {
        767, // Width
        1025, // Height
        4, // MipLevels
        1, // ArraySize
        1, // SubResourceCount
        {  // SubResources
            {0, 0}, // MipLevel, ArraySlice
            {0, 0}, // MipLevel, ArraySlice
            {0, 0}, // MipLevel, ArraySlice
            {0, 0}, // MipLevel, ArraySlice
        },
        0, // MiscFlags
    },
    {
        2048, // Width
        2048, // Height
        1, // MipLevels
        1, // ArraySize
        1, // SubResourceCount
        {  // SubResources
            {0, 0}, // MipLevel, ArraySlice
            {0, 0}, // MipLevel, ArraySlice
            {0, 0}, // MipLevel, ArraySlice
            {0, 0}, // MipLevel, ArraySlice
        },
        0, // MiscFlags
    },
};
UINT texture2DSizeCount = sizeof(texture2DSizes)/sizeof(texture2DSizes[0]);

const char *
texture2DPatterns[2][2] =
{
    {"aAbBcCdDeEfFgGhHiIjJ", "AaBbCcDdEeFfGgHhIiJj"},
    {"zZyYxXwWvVuUtTsSrRqQ", "ZzYyXxWwVvUuTtSsRrQq"},
};

void SubTestTexture2D(
    cl_context context,
    cl_command_queue command_queue,
    cl_kernel kernel,
    ID3D10Device* pDevice,
    const TextureFormat* format,
    const Texture2DSize* size)
{
    ID3D10Texture2D* pTexture = NULL;
    HRESULT hr = S_OK;

    cl_int result = CL_SUCCESS;

    HarnessD3D10_TestBegin("2D Texture: Format=%s, Width=%d, Height=%d, MipLevels=%d, ArraySize=%d",
        format->name_format,
        size->Width,
        size->Height,
        size->MipLevels,
        size->ArraySize);

    struct
    {
        cl_mem mem;
        UINT subResource;
        UINT width;
        UINT height;
    }
    subResourceInfo[4];

    cl_event events[4] = {NULL, NULL, NULL, NULL};

    // create the D3D10 resources
    {
        D3D10_TEXTURE2D_DESC desc;
        memset(&desc, 0, sizeof(desc) );
        desc.Width      = size->Width;
        desc.Height     = size->Height;
        desc.MipLevels  = size->MipLevels;
        desc.ArraySize  = size->ArraySize;
        desc.Format     = format->format;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D10_USAGE_DEFAULT;
        desc.BindFlags = D3D10_BIND_SHADER_RESOURCE | D3D10_BIND_RENDER_TARGET;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;

        hr = pDevice->CreateTexture2D(&desc, NULL, &pTexture);
        TestRequire(SUCCEEDED(hr), "ID3D10Device::CreateTexture2D failed (non-OpenCL D3D error, but test is invalid).");
    }

    // initialize some useful variables
    for (UINT i = 0; i < size->SubResourceCount; ++i)
    {
        // compute the expected values for the subresource
        subResourceInfo[i].subResource = D3D10CalcSubresource(
            size->subResources[i].MipLevel,
            size->subResources[i].ArraySlice,
            size->MipLevels);
        subResourceInfo[i].width = size->Width;
        subResourceInfo[i].height = size->Height;
        for (UINT j = 0; j < size->subResources[i].MipLevel; ++j) {
            subResourceInfo[i].width /= 2;
            subResourceInfo[i].height /= 2;
        }
    }

    // copy a pattern into the corners of the image, coordinates
    // (0,0), (w,0-1), (0,h-1), (w-1,h-1)
    for (UINT i = 0; i < size->SubResourceCount; ++i)
    for (UINT x = 0; x < 2; ++x)
    for (UINT y = 0; y < 2; ++y)
    {
        // create the staging buffer
        ID3D10Texture2D* pStagingBuffer = NULL;
        {
            D3D10_TEXTURE2D_DESC desc = {0};
            desc.Width      = 1;
            desc.Height     = 1;
            desc.MipLevels  = 1;
            desc.ArraySize  = 1;
            desc.Format     = format->format;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            desc.Usage = D3D10_USAGE_STAGING;
            desc.BindFlags = 0;
            desc.CPUAccessFlags = D3D10_CPU_ACCESS_READ | D3D10_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;
            hr = pDevice->CreateTexture2D(&desc, NULL, &pStagingBuffer);
            TestRequire(SUCCEEDED(hr), "ID3D10Device::CreateTexture2D failed (non-OpenCL D3D error, but test is invalid).");
        }

        // write the data to the staging buffer
        {
            D3D10_MAPPED_TEXTURE2D mappedTexture;
            hr = pStagingBuffer->Map(
                0,
                D3D10_MAP_READ_WRITE,
                0,
                &mappedTexture);
            memcpy(mappedTexture.pData, texture2DPatterns[x][y], format->bytesPerPixel);
            pStagingBuffer->Unmap(0);
        }

        // copy the data to to the texture
        {
            D3D10_BOX box = {0};
            box.front   = 0; box.back    = 1;
            box.top     = 0; box.bottom  = 1;
            box.left    = 0; box.right   = 1;
            pDevice->CopySubresourceRegion(
                pTexture,
                subResourceInfo[i].subResource,
                x ? subResourceInfo[i].width  - 1 : 0,
                y ? subResourceInfo[i].height - 1 : 0,
                0,
                pStagingBuffer,
                0,
                &box);
        }

        pStagingBuffer->Release();
    }

    // create the cl_mem objects for the resources and verify its sanity
    for (UINT i = 0; i < size->SubResourceCount; ++i)
    {
        // create a cl_mem for the resource
        subResourceInfo[i].mem = clCreateFromD3D10Texture2DKHR(
            context,
            0,
            pTexture,
            subResourceInfo[i].subResource,
            &result);
        TestRequire(result == CL_SUCCESS, "clCreateFromD3D10Texture2DKHR failed");

        // query resource pointer and verify
        ID3D10Resource* clResource = NULL;
        result = clGetMemObjectInfo(
            subResourceInfo[i].mem,
            CL_MEM_D3D10_RESOURCE_KHR,
            sizeof(clResource),
            &clResource,
            NULL);
        TestRequire(result == CL_SUCCESS, "clGetMemObjectInfo for CL_MEM_D3D10_RESOURCE_KHR failed.");
        TestRequire(clResource == pTexture, "clGetMemObjectInfo for CL_MEM_D3D10_RESOURCE_KHR returned incorrect value.");

        // query subresource and verify
        UINT clSubResource;
        result = clGetImageInfo(
            subResourceInfo[i].mem,
            CL_IMAGE_D3D10_SUBRESOURCE_KHR,
            sizeof(clSubResource),
            &clSubResource,
            NULL);
        TestRequire(result == CL_SUCCESS, "clGetImageInfo for CL_IMAGE_D3D10_SUBRESOURCE_KHR failed");
        TestRequire(clSubResource == subResourceInfo[i].subResource, "clGetImageInfo for CL_IMAGE_D3D10_SUBRESOURCE_KHR returned incorrect value.");

        // query format and verify
        cl_image_format clFormat;
        result = clGetImageInfo(
            subResourceInfo[i].mem,
            CL_IMAGE_FORMAT,
            sizeof(clFormat),
            &clFormat,
            NULL);
        TestRequire(result == CL_SUCCESS, "clGetImageInfo for CL_IMAGE_FORMAT failed");
        TestRequire(clFormat.image_channel_order == format->channel_order, "clGetImageInfo for CL_IMAGE_FORMAT returned incorrect channel order.");
        TestRequire(clFormat.image_channel_data_type == format->channel_type, "clGetImageInfo for CL_IMAGE_FORMAT returned incorrect channel data type.");

        // query width
        size_t width;
        result = clGetImageInfo(
            subResourceInfo[i].mem,
            CL_IMAGE_WIDTH,
            sizeof(width),
            &width,
            NULL);
        TestRequire(result == CL_SUCCESS, "clGetImageInfo for CL_IMAGE_WIDTH failed");
        TestRequire(width == subResourceInfo[i].width, "clGetImageInfo for CL_IMAGE_HEIGHT returned incorrect value.");

        // query height
        size_t height;
        result = clGetImageInfo(
            subResourceInfo[i].mem,
            CL_IMAGE_HEIGHT,
            sizeof(height),
            &height,
            NULL);
        TestRequire(result == CL_SUCCESS, "clGetImageInfo for CL_IMAGE_HEIGHT failed");
        TestRequire(height == subResourceInfo[i].height, "clGetImageInfo for CL_IMAGE_HEIGHT returned incorrect value.");

    }

    // acquire the resources for OpenCL
    for (UINT i = 0; i < 2; ++i)
    {
        cl_uint memCount = 0;
        cl_mem memToAcquire[MAX_REGISTERED_SUBRESOURCES];

        // cut the registered sub-resources into two sets and send the acquire calls for them separately
        if (i == 0)
        {
            for(UINT j = 0; j < size->SubResourceCount/2; ++j)
            {
                memToAcquire[memCount++] = subResourceInfo[j].mem;
            }
        }
        else
        {
            for(UINT j = size->SubResourceCount/2; j < size->SubResourceCount; ++j)
            {
                memToAcquire[memCount++] = subResourceInfo[j].mem;
            }
        }
        if (!memCount) continue;

        // do the acquire
        result = clEnqueueAcquireD3D10ObjectsKHR(
            command_queue,
            memCount,
            memToAcquire,
            0,
            NULL,
            &events[0+i]);
        TestRequire(result == CL_SUCCESS, "clEnqueueAcquireD3D10ObjectsKHR failed.");
        TestRequire(events[0+i], "clEnqueueAcquireD3D10ObjectsKHR did not return an event.");

        // make sure the event type is correct
        cl_uint eventType = 0;
        result = clGetEventInfo(
            events[0+i],
            CL_EVENT_COMMAND_TYPE,
            sizeof(eventType),
            &eventType,
            NULL);
        TestRequire(result == CL_SUCCESS, "clGetEventInfo for event created by clEnqueueAcquireD3D10ObjectsKHR failed.");
        TestRequire(eventType == CL_COMMAND_ACQUIRE_D3D10_OBJECTS_KHR, "clGetEventInfo for CL_EVENT_COMMAND_TYPE was not CL_COMMAND_ACQUIRE_D3D10_OBJECTS_KHR.");
    }

    // download the data using OpenCL & compare with the expected results
    for (UINT i = 0; i < size->SubResourceCount; ++i)
    {
        // copy (0,0) to (1,1) and (w-1,h-1) to (w-2,h-2) using a kernel
        {
            result = clSetKernelArg(
                kernel,
                0,
                sizeof(cl_mem),
                (void *)&subResourceInfo[i].mem);
            result = clSetKernelArg(
                kernel,
                1,
                sizeof(cl_mem),
                (void *)&subResourceInfo[i].mem);

            TestRequire(CL_SUCCESS == result, "clSetKernelArg failed");

            size_t localWorkSize[] = {1};
            size_t globalWorkSize[] = {1};
            result = clEnqueueNDRangeKernel(
                command_queue,
                kernel,
                1,
                NULL,
                globalWorkSize,
                localWorkSize,
                0,
                NULL,
                NULL);
            TestRequire(CL_SUCCESS == result, "clEnqueueNDRangeKernel failed");
        }
        // copy (w-1,0) to (w-2,1) and (0,h) to (1,h-2) using a memcpy
        for (UINT x = 0; x < 2; ++x)
        for (UINT y = 0; y < 2; ++y)
        {
            if (x == y)
            {
                continue;
            }

            size_t src[3] =
            {
                x ? subResourceInfo[i].width  - 1 : 0,
                y ? subResourceInfo[i].height - 1 : 0,
                0,
            };
            size_t dst[3] =
            {
                x ? subResourceInfo[i].width  - 2 : 1,
                y ? subResourceInfo[i].height - 2 : 1,
                0,
            };
            size_t region[3] =
            {
                1,
                1,
                1,
            };
            result = clEnqueueCopyImage(
                command_queue,
                subResourceInfo[i].mem,
                subResourceInfo[i].mem,
                src,
                dst,
                region,
                0,
                NULL,
                NULL);
            TestRequire(result == CL_SUCCESS, "clEnqueueCopyImage failed.");
        }
    }

    // release the resource from OpenCL
    for (UINT i = 0; i < 2; ++i)
    {
        cl_uint memCount = 0;
        cl_mem memToAcquire[MAX_REGISTERED_SUBRESOURCES];

        // cut the registered sub-resources into two sets and send the release calls for them separately
        if (i == 0)
        {
            for(UINT j = size->SubResourceCount/4; j < size->SubResourceCount; ++j)
            {
                memToAcquire[memCount++] = subResourceInfo[j].mem;
            }
        }
        else
        {
            for(UINT j = 0; j < size->SubResourceCount/4; ++j)
            {
                memToAcquire[memCount++] = subResourceInfo[j].mem;
            }
        }
        if (!memCount) continue;

        // do the release
        result = clEnqueueReleaseD3D10ObjectsKHR(
            command_queue,
            memCount,
            memToAcquire,
            0,
            NULL,
            &events[2+i]);
        TestRequire(result == CL_SUCCESS, "clEnqueueReleaseD3D10ObjectsKHR failed.");
        TestRequire(events[2+i], "clEnqueueReleaseD3D10ObjectsKHR did not return an event.");

        // make sure the event type is correct
        cl_uint eventType = 0;
        result = clGetEventInfo(
            events[2+i],
            CL_EVENT_COMMAND_TYPE,
            sizeof(eventType),
            &eventType,
            NULL);
        TestRequire(result == CL_SUCCESS, "clGetEventInfo for event created by clEnqueueReleaseD3D10ObjectsKHR failed.");
        TestRequire(eventType == CL_COMMAND_RELEASE_D3D10_OBJECTS_KHR, "clGetEventInfo for CL_EVENT_COMMAND_TYPE was not CL_COMMAND_RELEASE_D3D10_OBJECTS_KHR.");
    }

    for (UINT i = 0; i < size->SubResourceCount; ++i)
    for (UINT x = 0; x < 2; ++x)
    for (UINT y = 0; y < 2; ++y)
    {
        // create the staging buffer
        ID3D10Texture2D* pStagingBuffer = NULL;
        {
            D3D10_TEXTURE2D_DESC desc = {0};
            desc.Width      = 1;
            desc.Height     = 1;
            desc.MipLevels  = 1;
            desc.ArraySize  = 1;
            desc.Format     = format->format;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            desc.Usage = D3D10_USAGE_STAGING;
            desc.BindFlags = 0;
            desc.CPUAccessFlags = D3D10_CPU_ACCESS_READ | D3D10_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;
            hr = pDevice->CreateTexture2D(&desc, NULL, &pStagingBuffer);
            TestRequire(SUCCEEDED(hr), "Failed to create staging buffer.");
        }

        // wipe out the staging buffer to make sure we don't get stale values
        {
            D3D10_MAPPED_TEXTURE2D mappedTexture;
            hr = pStagingBuffer->Map(
                0,
                D3D10_MAP_READ_WRITE,
                0,
                &mappedTexture);
            TestRequire(SUCCEEDED(hr), "Failed to map staging buffer");
            memset(mappedTexture.pData, 0, format->bytesPerPixel);
            pStagingBuffer->Unmap(0);
        }

        // copy the pixel to the staging buffer
        {
            D3D10_BOX box = {0};
            box.left    = x ? subResourceInfo[i].width  - 2 : 1; box.right  = box.left + 1;
            box.top     = y ? subResourceInfo[i].height - 2 : 1; box.bottom = box.top + 1;
            box.front   = 0;                                     box.back   = 1;
            pDevice->CopySubresourceRegion(
                pStagingBuffer,
                0,
                0,
                0,
                0,
                pTexture,
                subResourceInfo[i].subResource,
                &box);
        }

        // make sure we read back what was written next door
        {
            D3D10_MAPPED_TEXTURE2D mappedTexture;
            hr = pStagingBuffer->Map(
                0,
                D3D10_MAP_READ_WRITE,
                0,
                &mappedTexture);
            TestRequire(SUCCEEDED(hr), "Failed to map staging buffer");

            /*
            // This can be helpful in debugging...
            printf("\n");
            for (UINT k = 0; k < format->bytesPerPixel; ++k)
            {
                printf("[%c %c]\n",
                    texture2DPatterns[x][y][k],
                    ( (char *)mappedTexture.pData )[k]);
            }
            */

            TestRequire(
                !memcmp(mappedTexture.pData, texture2DPatterns[x][y], format->bytesPerPixel),
                "Failed to map staging buffer");

            pStagingBuffer->Unmap(0);
        }

        pStagingBuffer->Release();
    }


Cleanup:

    if (pTexture)
    {
        pTexture->Release();
    }
    for (UINT i = 0; i < size->SubResourceCount; ++i)
    {
        clReleaseMemObject(subResourceInfo[i].mem);
    }
    for (UINT i = 0; i < 4; ++i)
    {
        if (events[i])
        {
            result = clReleaseEvent(events[i]);
            TestRequire(result == CL_SUCCESS, "clReleaseEvent for event failed.");
        }
    }

    HarnessD3D10_TestEnd();
}

void TestDeviceTexture2D(
    cl_device_id device,
    cl_context context,
    cl_command_queue command_queue,
    ID3D10Device* pDevice)
{
    cl_int result = CL_SUCCESS;
    cl_kernel kernels[3] = {NULL, NULL, NULL};

    const char *sourceRaw =
        " \
        __kernel void texture2D\n\
        ( \n\
            __read_only  image2d_t texIn, \n\
            __write_only image2d_t texOut \n\
        ) \n\
        { \n\
            const sampler_t smp = CLK_FILTER_NEAREST; \n\
            %s value;  \n\
            int2 coordIn;  \n\
            int2 coordOut; \n\
            int w = get_image_width(texIn); \n\
            int h = get_image_height(texIn); \n\
            \n\
            coordIn  = (int2)(0, 0); \n\
            coordOut = (int2)(1, 1); \n\
            value = read_image%s(texIn, smp, coordIn); \n\
            write_image%s(texOut, coordOut, value); \n\
            \n\
            coordIn  = (int2)(w-1, h-1); \n\
            coordOut = (int2)(w-2, h-2); \n\
            value = read_image%s(texIn, smp, coordIn); \n\
            write_image%s(texOut, coordOut, value); \n\
        } \n\
        ";

    char source[2048];
    sprintf(source, sourceRaw, "float4", "f", "f", "f", "f");
    result = HarnessD3D10_CreateKernelFromSource(&kernels[0], device, context, source, "texture2D");
    TestRequire(CL_SUCCESS == result, "HarnessD3D10_CreateKernelFromSource failed.");

    sprintf(source, sourceRaw, "uint4", "ui", "ui", "ui", "ui");
    result = HarnessD3D10_CreateKernelFromSource(&kernels[1], device, context, source, "texture2D");
    TestRequire(CL_SUCCESS == result, "HarnessD3D10_CreateKernelFromSource failed.");

    sprintf(source, sourceRaw, "int4", "i", "i", "i", "i");
    result = HarnessD3D10_CreateKernelFromSource(&kernels[2], device, context, source, "texture2D");
    TestRequire(CL_SUCCESS == result, "HarnessD3D10_CreateKernelFromSource failed.");

    for (UINT format = 0, size = 0; format < formatCount; ++size, ++format)
    {
        SubTestTexture2D(
            context,
            command_queue,
            kernels[formats[format].generic],
            pDevice,
            &formats[format],
            &texture2DSizes[size % texture2DSizeCount]);
    }

Cleanup:

    for (UINT i = 0; i < 3; ++i)
    {
        if (kernels[i])
        {
            clReleaseKernel(kernels[i]);
        }
    }
}

