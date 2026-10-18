import axios from "axios";
import tokenService from "../service/tokenService";

const api = axios.create({
  baseURL: import.meta.env.VITE_API_BASE_URL || "http://localhost:8022/api/v1",
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

// Gérer les erreurs d'authentification et d'autorisation
api.interceptors.response.use(
  (response) => response,
  (error) => {
    const status = error.response?.status;
    const requestUrl = error.config?.url ?? "";

    const isLoginRequest = requestUrl.includes("/auth/login");
    const isAuthError = status === 401 || status === 403;
    const isAlreadyOnSignIn = window.location.pathname === "/signin";

    if (isAuthError && !isLoginRequest && !isAlreadyOnSignIn) {
      tokenService.removeToken();
      window.location.replace("/signin");
    }

    return Promise.reject(error);
  },
);

export default api;
