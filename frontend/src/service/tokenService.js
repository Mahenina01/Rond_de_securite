const TOKEN_KEY = "access_token";

const tokenService = {
  getToken: () => sessionStorage.getItem(TOKEN_KEY),

  setToken: (token) => {
    sessionStorage.setItem(TOKEN_KEY, token);
  },

  removeToken: () => {
    sessionStorage.removeItem(TOKEN_KEY);
  },

  isTokenValid: () => {
    const token = sessionStorage.getItem(TOKEN_KEY);

    if (!token) return false;

    try {
      const payload = token.split(".")[1];

      if (!payload) return false;

      const base64 = payload.replace(/-/g, "+").replace(/_/g, "/");

      const decoded = JSON.parse(
        decodeURIComponent(
          Array.from(atob(base64))
            .map(
              (char) => `%${char.charCodeAt(0).toString(16).padStart(2, "0")}`,
            )
            .join(""),
        ),
      );

      // Le JWT doit contenir une date d'expiration
      if (typeof decoded.exp !== "number") return false;

      return decoded.exp * 1000 > Date.now();
    } catch {
      return false;
    }
  },
};

export default tokenService;
