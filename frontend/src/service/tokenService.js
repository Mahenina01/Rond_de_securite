const TOKEN_KEY = "access_token";

const tokenService = {
  getToken: () => sessionStorage.getItem(TOKEN_KEY),

  setToken: (token) => {
    sessionStorage.setItem(TOKEN_KEY, token);
  },

  removeToken: () => {
    sessionStorage.removeItem(TOKEN_KEY);
  },
};

export default tokenService;
