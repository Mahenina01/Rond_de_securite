import axios from "axios";
import tokenService from "../service/tokenService";

const api = axios.create({
  baseURL: import.meta.env.API_BASE_URL || "http://localhost:8022/api/v1",
  headers: {
    "Content-Type": "application/json",
  },
});

// Ajouter automatiquement le token aux requêtes
api.interceptors.request.use(
  (config) => {
    const token = tokenService.getToken();

    if (token) {
      config.headers.Authorization = `Bearer ${token}`;
    }

    return config;
  },
  (error) => Promise.reject(error),
);

// Gérer les tokens invalides ou expirés
api.interceptors.response.use(
  (response) => response,
  (error) => {
    const status = error.response?.status;
    const requestUrl = error.config?.url ?? "";
    const isLoginRequest = requestUrl.includes("/auth/login");

    if (
      status === 401 &&
      !isLoginRequest &&
      window.location.pathname !== "/signin"
    ) {
      tokenService.removeToken();

      window.location.replace("/signin");
    }

    return Promise.reject(error);
  },
);

export default api;
