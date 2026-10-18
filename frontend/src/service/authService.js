import api from "../api/axios";
import endpoints from "../api/endpoints";
import tokenService from "./tokenService";

const authService = {
  login: async (credentials) => {
    const response = await api.post(endpoints.auth.login, credentials);

    const data = response.data;

    // Adaptez cette propriété au JSON réel du backend.
    const token = data.accessToken ?? data.token;

    if (!token) {
      throw new Error("Connexion réussie, mais aucun token JWT n'a été reçu.");
    }

    tokenService.setToken(token);

    return data;
  },

  logout: async () => {
    try {
      const response = await api.post(endpoints.auth.logout);
      return response.data;
    } finally {
      tokenService.removeToken();
    }
  },

  getMe: async () => {
    const response = await api.get(endpoints.auth.me);
    return response.data;
  },

  getProfile: async () => {
    const response = await api.get(endpoints.auth.profile);
    return response.data;
  },

  updateProfile: async (profileData) => {
    const response = await api.patch(endpoints.auth.profile, profileData);
    return response.data;
  },

  changePassword: async (passwordData) => {
    const response = await api.patch(endpoints.auth.password, passwordData);
    return response.data;
  },
};

export default authService;
