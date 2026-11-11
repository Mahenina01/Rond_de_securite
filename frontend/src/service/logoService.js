import api from "../api/axios";
import endpoints from "../api/endpoints";

const API_BASE_URL =
  import.meta.env.VITE_API_BASE_URL || "http://localhost:8022/api/v1";

const logoService = {
  getCurrent: async () => {
    const response = await api.get(endpoints.logo.current);
    return response.data;
  },

  save: async (file, exists) => {
    const formData = new FormData();
    formData.append("file", file);

    const response = await api.request({
      method: exists ? "put" : "post",
      url: exists ? endpoints.logo.update : endpoints.logo.create,
      data: formData,
      headers: {
        "Content-Type": "multipart/form-data",
      },
    });

    return response.data;
  },

  getImageUrl: async (photoPath) => {
    if (!photoPath) return null;
    const imageUrl = new URL(photoPath, new URL(API_BASE_URL).origin).href;
    const response = await api.get(imageUrl, { responseType: "blob" });
    return URL.createObjectURL(response.data);
  },
};

export default logoService;
