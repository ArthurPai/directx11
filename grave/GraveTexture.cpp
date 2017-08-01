#include "GraveTexture.h"

GraveTexture::GraveTexture()
{
    m_targaData = 0;
    m_texture = 0;
    m_textureView = 0;
}

GraveTexture::~GraveTexture()
{
}

bool GraveTexture::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
    bool result;
    int height, width;
    D3D11_TEXTURE2D_DESC textureDesc;
    HRESULT hResult;
    unsigned int rowPitch;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

    // 載入貼圖檔案
    result = LoadTarga(filename, height, width);
    if (!result)
    {
        return false;
    }

    // 描述貼圖資訊
    textureDesc.Height = height;
    textureDesc.Width = width;
    textureDesc.MipLevels = 0;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    // 建立空的貼圖
    hResult = device->CreateTexture2D(&textureDesc, NULL, &m_texture);
    if (FAILED(hResult))
    {
        return false;
    }

    // 計算圖片記憶體大小 (row pitch)
    rowPitch = (width * 4) * sizeof(unsigned char);

    // 將圖片複製到貼圖資源
    deviceContext->UpdateSubresource(m_texture, 0, NULL, m_targaData, rowPitch, 0);

    // 描述 shader resource view 資訊
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = -1;

    // 建立貼圖的 shader resource view
    hResult = device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureView);
    if (FAILED(hResult))
    {
        return false;
    }

    // 產生 mipmaps
    deviceContext->GenerateMips(m_textureView);

    // 釋放記憶體
    delete[] m_targaData;
    m_targaData = 0;

    return true;
}

void GraveTexture::Shutdown()
{
    if (m_textureView)
    {
        m_textureView->Release();
        m_textureView = 0;
    }

    if (m_texture)
    {
        m_texture->Release();
        m_texture = 0;
    }

    if (m_targaData)
    {
        delete[] m_targaData;
        m_targaData = 0;
    }
}

ID3D11ShaderResourceView* GraveTexture::GetTexture()
{
    return m_textureView;
}

bool GraveTexture::LoadTarga(char* filename, int& height, int& width)
{
    int error, bpp, imageSize, index, i, j, k;
    FILE* filePtr;
    unsigned int count;
    TargaHeader targaFileHeader;
    unsigned char* targaImage;

    // 開啟檔案
    error = fopen_s(&filePtr, filename, "rb");
    if (error != 0)
    {
        return false;
    }

    // 讀取檔案標頭資訊
    count = (unsigned int)fread(&targaFileHeader, sizeof(TargaHeader), 1, filePtr);
    if (count != 1)
    {
        return false;
    }

    // 獲取圖檔資訊
    height = (int)targaFileHeader.height;
    width = (int)targaFileHeader.width;
    bpp = (int)targaFileHeader.bpp;

    // 檢查是不是32bit R8G8B8A8
    if (bpp != 32)
    {
        return false;
    }

    // 計算記憶體大小
    imageSize = width * height * 4;

    // 配置記憶體
    targaImage = new unsigned char[imageSize];
    if (!targaImage)
    {
        return false;
    }

    // 讀取圖片
    count = (unsigned int)fread(targaImage, 1, imageSize, filePtr);
    if (count != imageSize)
    {
        return false;
    }

    // 關檔
    error = fclose(filePtr);
    if (error != 0)
    {
        return false;
    }

    // 配置記憶體
    m_targaData = new unsigned char[imageSize];
    if (!m_targaData)
    {
        return false;
    }

    // Initialize the index into the targa destination data array.
    index = 0;

    // Initialize the index into the targa image data.
    k = (width * height * 4) - (width * 4);

    // Now copy the targa image data into the targa destination array in the correct order since the targa format is stored upside down.
    for (j = 0; j<height; j++)
    {
        for (i = 0; i<width; i++)
        {
            m_targaData[index + 0] = targaImage[k + 2];  // Red.
            m_targaData[index + 1] = targaImage[k + 1];  // Green.
            m_targaData[index + 2] = targaImage[k + 0];  // Blue
            m_targaData[index + 3] = targaImage[k + 3];  // Alpha

            // Increment the indexes into the targa data.
            k += 4;
            index += 4;
        }

        // Set the targa image data index back to the preceding row at the beginning of the column since its reading it in upside down.
        k -= (width * 8);
    }

    // Release the targa image data now that it was copied into the destination array.
    delete[] targaImage;
    targaImage = 0;

    return true;
}